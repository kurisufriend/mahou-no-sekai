#include "backend.h"
#include "../lib/dumbstr/dumbstr.h"

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
    "no INTEGER"
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
            "\"", name, "\",",
            "\"", topic, "\",",
            "\"", flavor, "\",",
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
            "\"", board, "\",",
            std::to_string(no), ",",
            "\"", subject, "\",",
            std::to_string(replies), ",",
            std::to_string(bump_time),
            ");"
        })
    }.exec();
}

void be::make_post(sqlite3 *db, std::string board, int op, std::string body, std::string name, std::string trip, int time, int no)
{
    sqleasy_q{db,
        dumbfmt({
            "insert into posts values(",
            "\"", board, "\",",
            std::to_string(op), ",",
            "\"", name, "\",",
            "\"", trip, "\",",
            "\"", body, "\",",
            std::to_string(time), ",",
            std::to_string(no),
            ");"
        })
    }.exec();
}

std::vector<std::string> be::get_boards(sqlite3* db)
{
    rows rs = sqleasy_q{db, "select * from boards;"}.exec();
    std::vector<std::string> res;
    foreach(rs, r)
        {res.push_back((*r)["name"]);}
    return res;
}