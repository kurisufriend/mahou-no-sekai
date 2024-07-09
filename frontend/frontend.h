#pragma once
#include <string>
#include "../lib/sqleasy/sqleasy.h"

namespace fe 
{
    std::string generate_board(sqlite3* db, std::string name, int page = 0, bool cache = true);
}