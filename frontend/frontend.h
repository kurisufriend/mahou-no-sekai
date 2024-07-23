#pragma once
#include <string>
#include <map>
#include "../lib/sqleasy/sqleasy.h"

namespace fe 
{
    std::string generate_board(sqlite3* db, std::string name, std::map<std::string, std::string> &qstring, int threadid = -1, bool cache = true);
    std::string generate_index(sqlite3* db, std::string board, std::map<std::string, std::string> &qstring, bool catalog = false, bool cache = true);
    std::string generate_thread(sqlite3* db, std::string board, int op, bool cache = false);
}