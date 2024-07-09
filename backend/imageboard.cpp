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
    "subject TEXT,"
    "replies INTEGER"
    ");";
    const char* post_schema = "create table posts("
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

std::vector<std::string> be::get_boards(sqlite3* db)
{
    rows rs = sqleasy_q{db, "select * from boards;"}.exec();
    std::vector<std::string> res;
    foreach(rs, r)
        {res.push_back((*r)["name"]);}
    return res;
}