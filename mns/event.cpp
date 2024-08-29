#include "event.h"
#include "../lib/phile/phile.h"
#include "../lib/dumbstr/dumbstr.h"
#include "../lib/json.hpp"
#include <vector>
#include <string>
#include <sys/stat.h>
#include <iostream>
#include "../backend/backend.h"
#include "../frontend/frontend.h"

void mns::evmanager::load(std::string eventlogpath)
{
    if (!ph::folder_exists(eventlogpath))
        mkdir(eventlogpath.c_str(), 0733);
    if (!ph::file_exists(eventlogpath + "/head"))
        ph::file_put_contents(eventlogpath + "/head", "0");
    if (!ph::file_exists(eventlogpath + "/log"))
        ph::file_put_contents(eventlogpath + "/log", "0\tnop\thello world~\n");
    std::string head = dumbfmt_file(eventlogpath + "/head", {}, false);
    std::string log = dumbfmt_file(eventlogpath + "/log", {}, false);
    this->logpath = eventlogpath;
    this->last_processed_event = std::stoi(head);
    this->update(log);
}

void mns::evmanager::update(std::string& eventlogcontents)
{
    std::vector<std::string> lines = dumbfmt_split(eventlogcontents, '\n');
    foreach (lines, i)
    {
        if (*i == "")
            {continue;}
        this->process(*i);
    }
}

void mns::evmanager::log(std::string& event)
{
    ph::file_put_contents(this->logpath+"/log", event + "\n", true);
}

void mns::evmanager::process(std::string& event)
{
    std::vector<std::string> data = dumbfmt_split(event, '\t');
    if (data.size() < 3)
        return;
    int id = -1;
    try 
    {
        id = stoi(data.at(0));
    } 
    catch (...) 
    {
        std::cout << "event " << event << " invalid id" << std::endl;
        return;
    }
    if (id <= this->last_processed_event)
    {
        std::cout << "event " << event << " already processed" << std::endl;
        return;
    }


    if (data.at(1) == "nop")
    {
        // do a little dance
    }
    else if (data.at(1) == "post")
    {
        nlohmann::json j = nlohmann::json::parse(data.at(2));
        be::make_post(this->db,
            j["board"],
            j["op"],
            j["body"],
            j["name"],
            j["trip"],
            j["time"],
            j["no"],
            j["filename"],
            j["uploadname"],
            j["sage"]
        );

        if (j["op"] == j["no"])
        {
            be::make_thread(db,
                j["board"],
                j["subject"],
                j["no"],
                0,
                j["time"]
            );
        }

        sqleasy_q{db, dumbfmt({"update boards set no=",std::to_string((int)(j["no"]))," where name=\"",se(j["board"]),"\";"})}.exec();

        // reload frontend caches
        this->gc->update_board(db, j["board"]);
        this->gc->update_thread_index(db, j["board"], j["op"]);
    }


    std::cout << event << std::endl;
    this->last_processed_event = id;
    ph::file_put_contents(this->logpath+"/head", std::to_string(this->last_processed_event));
}

void mns::evmanager::create(std::string event)
{
    this->process(event);
    this->log(event);
}