#include "backend.h"
#include "../lib/dumbstr/dumbstr.h"
#include "../lib/tinternet/tinternet.h"
#include "../lib/women/women.h"
#include "../lib/json.hpp"
#include <iterator>

void be::init_database_schemae(sqlite3* db)
{
    const char* board_schema = "create table boards("
    "name TEXT,"
    "topic TEXT,"
    "flavor TEXT,"
    "no INTEGER"
    ");";
    const char* thread_schema = "create table threads("
    "board TEXT,"
    "no INTEGER,"
    "subject TEXT,"
    "replies INTEGER,"
    "bump_time INTEGER"
    ");";
    const char* post_schema = "create table posts("
    "board TEXT,"
    "op INTEGER,"
    "name TEXT,"
    "trip TEXT,"
    "body TEXT,"
    "time INTEGER,"
    "no INTEGER,"
    "filename TEXT,"
    "uploadname TEXT"
    ");";

    sqleasy_q{db, board_schema}.exec();
    sqleasy_q{db, thread_schema}.exec();
    sqleasy_q{db, post_schema}.exec();
}

void be::make_board(sqlite3* db, std::string name, std::string topic, std::string flavor, int post_no)
{
    sqleasy_q{db,
        dumbfmt({
            "insert into boards values(",
            "\"", se(name), "\",",
            "\"", se(topic), "\",",
            "\"", se(flavor), "\",",
            std::to_string(post_no),
            ");"
        })
    }.exec();
}

void be::make_thread(sqlite3 *db, std::string board, std::string subject, int no, int replies, int bump_time)
{
    sqleasy_q{db,
        dumbfmt({
            "insert into threads values(",
            "\"", se(board), "\",",
            std::to_string(no), ",",
            "\"", se(subject), "\",",
            std::to_string(replies), ",",
            std::to_string(bump_time),
            ");"
        })
    }.exec();
}

void be::make_post(sqlite3 *db, std::string board, int op, std::string body, std::string name, std::string trip, int time, int no, std::string filename, std::string uploadname, bool sage)
{
    sqleasy_q{db,
        dumbfmt({
            "insert into posts values(",
            "\"", se(board), "\",",
            se(std::to_string(op)), ",",
            "\"", se(name), "\",",
            "\"", se(trip), "\",",
            "\"", se(body), "\",",
            std::to_string(time), ",",
            std::to_string(no), ",",
            "\"", se(filename), "\",",
            "\"", se(uploadname), "\"",
            ");"
        })
    }.exec();
    if (!sage)
        be::bump(db, board, op);
    be::update_reply_count(db, board, op);
}

std::vector<std::string> be::get_boards(sqlite3* db)
{
    rows rs = sqleasy_q{db, "select * from boards;"}.exec();
    std::vector<std::string> res;
    foreach(rs, r)
        {res.push_back((*r)["name"]);}
    return res;
}

void be::bump(sqlite3* db, std::string board, int no)
{
    time_t t = time(0);
    sqleasy_q{db, dumbfmt({"update threads set bump_time=",std::to_string(t)," where no=",std::to_string(no)," and board=\"",se(board),"\";"})}.exec();
}

void be::update_reply_count(sqlite3 *db, std::string board, int no)
{
    int replies = std::stoi(sqleasy_q{db, dumbfmt({"select count(*) from posts where op=",std::to_string(no)})}.exec().at(0).begin()->second);
    sqleasy_q{db, dumbfmt({"update threads set replies=",std::to_string(--replies)," where no=",std::to_string(no)," and board=\"",se(board),"\";"})}.exec();
}

/*
the first line of defense for a submitted post.
returns: whether the post attempt is good to go, which
 determines whether the image gets savead and what gets signalled
 to the user.

this should not actually ever call make_post -- that remains the responsibility of the
 event system. instead, we make sure an event /should/ be made.
what should prevent a post event from making it to the event system?
- the poster is banned (not yet implemented)
- the thread or board is ASN locked, and the poster has an external IP (not yet implemented)
- the captcha is incorrect (not yet implemented)

- the thread being posted to does not exist
- the board being posted to does not exist
- the subject/name/body is over limits -- should be in settings

excepting these circumstances, this should then generate the event and return true.
*/
be::err be::handle_post_attempt(sqlite3* db, mns::evmanager* e,
    std::string board,
    std::string thread,
    std::string subject,
    std::string name,
    std::string body,
    std::string filename,
    std::string uploadname,
    std::string challenge_token,
    std::string challenge_response,
    std::string sage,
    std::string flags,
    std::string asnlock)
{

    if (name == "")
        name = "Anonymous";
    // TODO default name as board setting
    std::string trip = "";
    std::vector<std::string> t = dumbfmt_split(name, '!');
    std::vector<std::string> y;
    std::copy_if(t.begin(), t.end(), std::back_inserter(y), [](std::string s){return s != "";});
    if (y.size() > 1)
    {
        name = y.at(0);
        trip = y.at(1);
        //not futaba-style, i know. whadda ya gonna do about it? at least it's secure-by-default.
        std::string ssalt = e->cfg["secret_salt"];
        std::string hash = sha256(ssalt+trip);
        trip = "!"+(hash.substr(0, 10));
    }
    else
    {
        trip = "";
    }
    //TODO make these a board setting
    if (subject.length() > 50)
        return be::err(-4, "subject too long");
    if (name.length() > 20)
        return be::err(-3, "name too long");
    if (body.length() > 1500)
        return be::err(-2, "body too long");
    if (thread == "-1" && filename == "")
        return (be::err(-5, "OP post in new thread must have an image"));

    nlohmann::json j;
    int bcount = std::stoi(sqleasy_q{db, dumbfmt({"select count(*) from boards where name=\"",se(board),"\""})}.exec().at(0).begin()->second);
    if (!bcount)
        return be::err(-0, "invalid board!");
    j["board"] = board;

    int ithread = -1337;
    ithread = sstoi(thread);
    int tcount = std::stoi(sqleasy_q{db, dumbfmt({"select count(*) from threads where no=",se(thread)})}.exec().at(0).begin()->second);
    if ((!tcount && ithread != -1) || ithread < -1)
        return be::err(0, "invalid thread!");
    j["op"] = ithread;
    
    j["name"] = dumbfmt_html_escape(name);
    j["trip"] = trip;
    const std::vector<std::string> reps = {"\r\n", "\n\r", "\r", "\n"};
    foreach(reps, nigs)
        body = dumbfmt_replace(*nigs, "\n", body);
    body = dumbfmt_html_escape(body);
    body = dumbfmt_replace("\n", "<br>", body);
    j["body"] = body;

    j["subject"] = dumbfmt_html_escape(subject);

    time_t tim;
    j["time"] = time(0);

    int post_no = std::stoi(sqleasy_q{db, dumbfmt({"select count(*) from posts where board=\"",se(board),"\""})}.exec().at(0).begin()->second);
    
    j["no"] = ++(post_no);
    if (j["op"] == -1)
        j["op"] = j["no"];

    uploadname = dumbfmt_html_escape(uploadname);
    j["uploadname"] = uploadname;
    j["filename"] = filename;

    j["sage"] = sage == "on";
    j["flags"] = flags == "on";
    j["asnlock"] = asnlock;


    e->create(dumbfmt({std::to_string(e->last_processed_event+1), "post", j.dump()}, "\t"));
    return err(1, "post event triggered.");
}