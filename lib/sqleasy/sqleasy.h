#pragma once
#include "../sqlite/sqlite3.h"
#include <map>
#include <vector>
#include <string>

typedef std::map<std::string, std::string/*std::pair<unsigned char, void*>*/> row;
typedef std::vector<row> rows;

#define qexec(q) sqleasy_q{db, q}.exec()

struct sqleasy_q//uery
{
    sqlite3* db;
    std::string query;
    char* err;

    static int cb(void* a, int argc, char** argv, char** colname);

    rows exec();
    void rexec(rows* res);
};