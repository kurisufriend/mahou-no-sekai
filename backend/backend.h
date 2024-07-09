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

    std::vector<std::string> get_boards(sqlite3* db);
}