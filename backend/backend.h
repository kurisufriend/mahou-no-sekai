#pragma once 
#include <vector>
#include <string>
#include "../lib/sqleasy/sqleasy.h"


namespace be
{
    //banners.cpp
    std::vector<std::string> get_banners(bool cache = true);
    std::string select_banner();

    //imageboard.cpp
    void init_database_schemae(sqlite3* db);
    void make_board(sqlite3* db, std::string name, std::string topic, std::string flavor, int post_no);
    void make_thread(sqlite3* db,
        std::string board,
        std::string subject = "",
        int no = -1, 
        int replies = 0, 
        int bump_time = -1
    );
    void make_post(sqlite3* db,
        std::string board,
        int op,
        std::string body,
        std::string name = "Anonymous",
        std::string trip = "",
        int time = -1,
        int no = -1
    );

    std::vector<std::string> get_boards(sqlite3* db);

    //web.cpp
    std::map<std::string, std::string> getquery(std::string raw);
}