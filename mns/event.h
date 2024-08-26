#pragma once
#include <string>
#include <vector>
#include "../lib/sqlite/sqlite3.h"
#include "cache.h"
#include "../lib/json.hpp"

// event format
// ---
//<id/int>\t<type/str>\t<data/str>
// ---

// events type, data
// post, json encoded post data w all fields the sql table has 

//TODO add 'origin' field for federation, count event ids from diff serbs separately to
// avoid ordering and timing issues

namespace mns
{
    struct evmanager
    {
        //keeps useful state 

        //post counter
        int post_no = 0;
        sqlite3* db;
        gcache* gc;
        nlohmann::json cfg;


        int last_processed_event = -1;
        std::string logpath;
        // loads the event files from disk, or creates them fresh if there is none, updating afterwards
        void load(std::string eventlogpath);

        // process all new events in log on disk
        void update(std::string& eventlogcontents);

        // adds event to log on disk and sends it to peers if needed
        void log(std::string& event);

        // enact the state change outlined by the event
        void process(std::string& event);

        // processes the passed event, then logs it (in that order)
        void create(std::string event);
    };
}