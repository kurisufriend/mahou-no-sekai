#pragma once
#include <string>
#include <map>
#include "../lib/sqleasy/sqleasy.h"
#include "../mns/cache.h"

namespace fe 
{
    std::string generate_board(sqlite3* db, std::string name, std::map<std::string, std::string> &qstring, mns::gcache* gcache, int threadid = -1);
    std::string generate_index(sqlite3* db, std::string board, std::map<std::string, std::string> &qstring, mns::gcache* gcache, bool catalog = false);
    std::string generate_thread(sqlite3* db, std::string board, int op, mns::gcache* gcache);
}