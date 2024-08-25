#pragma once
#include <map>
#include "../lib/sqleasy/sqleasy.h"

namespace mns
{
    struct gcache
    {
        // maps board name to board row
        std::map<std::string, row>  board_info_cache;

        // maps board name to [...threads]
        std::map<std::string, rows> board_cache;

        // maps (board, op) to (thread row, reply post rows)
        std::map<std::pair<std::string, int>, std::pair<row, rows>> index_cache;

        void update_board_info(sqlite3* db, std::string board);
        void update_board(sqlite3* db, std::string board);
        void update_thread_index(sqlite3* db, std::string board, int op);
    };
}