#include "cache.h"
#include "../lib/dumbstr/dumbstr.h"
#include <utility>

void mns::gcache::update_board_info(sqlite3* db, std::string board)
{
    rows r = sqleasy_q{db, dumbfmt({"select * from boards where name=\"", board, "\" limit 1;"})}.exec();
    row data = r.at(0);
    this->board_info_cache[board] = data;
}

void mns::gcache::update_board(sqlite3* db, std::string board)
{
    rows threads = sqleasy_q{db, dumbfmt({"select * from threads where board=\"",board,"\" order by bump_time desc;"})}.exec();
    this->board_cache[board] = threads;
}

void mns::gcache::update_thread_index(sqlite3 *db, std::string board, int op)
{
    rows threads = sqleasy_q{db, dumbfmt({"select * from threads where no=",std::to_string(op)," and board=\"",board,"\";"})}.exec();
    row thread = !threads.empty() ? threads.at(0) : row();

    rows posts = sqleasy_q{db, dumbfmt({"select * from posts where op=",std::to_string(op)," and board=\"",board,"\";"})}.exec();

    this->index_cache[std::pair<std::string, int>(board, op)] = std::pair<row, rows>(thread, posts);
}