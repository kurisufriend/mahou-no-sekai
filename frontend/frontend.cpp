#include "frontend.h"
#include "../backend/backend.h"
#include "../lib/dumbstr/dumbstr.h"
#include <vector>

std::string fe::generate_board(sqlite3* db, std::string name, int page, bool docache)
{
    static std::map<std::pair<std::string, int>, row> cache;

    row board;
    std::string res;

    if (docache && cache.find(std::pair<std::string, int>(name, page)) != cache.end())
    {
        board = cache.at(std::pair<std::string, int>(name, page));
    }
    else
    {
        rows r = sqleasy_q{db, dumbfmt({"select * from boards where name=\"", name, "\" limit 1;"})}.exec();
        board = r.at(0);
        cache.emplace(std::pair<std::string, int>(name, page), board);
    }

    std::vector<std::string> boards = be::get_boards(db);
    std::vector<std::string> links;
    foreach(boards, i)
    {
        links.push_back(dumbfmt({"<a href='/",*i,"'>",*i,"</a>"}));
    }

    return dumbfmt_file("./static/board.html", {
        {
            {"boardname", name},
            {"boardtopic", board["topic"]},
            {"boardflavor", board["flavor"]},
            {"banner source", dumbfmt({"/banners/", be::select_banner()})},
            {"top board list", dumbfmt({"[",dumbfmt(links, " / "),"]"})}
        }
    });
}
