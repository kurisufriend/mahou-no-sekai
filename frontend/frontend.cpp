#include "frontend.h"
#include "../backend/backend.h"
#include "../lib/dumbstr/dumbstr.h"
#include <string>
#include <vector>

std::string fe::generate_board(sqlite3* db, std::string name, std::map<std::string, std::string> &qstring, int page, bool docache)
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
            {"top board list", dumbfmt({"[",dumbfmt(links, " / "),"]"})},
            {"threads", fe::generate_index(db, name, qstring, false)}
        }
    });
}

std::string fe::generate_index(sqlite3 *db, std::string board, std::map<std::string, std::string> &qstring, bool catalog, bool cache)
{
    int page = 1;
    if (qstring.find("pg") != qstring.end())
    {
        try
        {
            page = std::stoi(qstring["pg"]);
        }
        catch (...)
        {}
    }
    rows r = sqleasy_q{db, dumbfmt({"select * from threads where board=\"",board,"\" order by bump_time desc limit 10 offset ",std::to_string((page-1) * 10),";"})}.exec();
    int count = std::stoi(sqleasy_q{db, dumbfmt({"select count(*) from threads where board=\"",board,"\""})}.exec().at(0).begin()->second);
    int pages = int(count/10)+1;
    std::string res = dumbfmt({std::to_string(count), " threads currently on-board, ", std::to_string(pages), " pages.", "<hr>"});
    foreach (r, i)
    {
        rows post_ = sqleasy_q{db, dumbfmt({"select * from posts where op=\"",(*i)["no"],"\""})}.exec();
        if (post_.empty())
            return "";
        foreach(post_, pos)
        {
            row post = *pos;
            time_t ti = (time_t)std::stoi(post["time"]);
            bool is_op = post["no"] == post["op"];
            res.append(
                dumbfmt_file("./static/template/post.html", {
                    {"no", post["no"]},
                    {"subject", is_op ? (*i)["subject"] : ""},
                    {"name", post["name"]},
                    {"trip", post["trip"]},
                    {"posterid", ""},
                    {"date", ctime(&ti)},
                    {"image", ""},
                    {"body", post["body"]},
                    {"attrs", is_op ? "" : "replypost"}
                })
            );
        }
        res.append("<hr>");
    }
    res.append("Pages: ");
    for (int i = 1; i <= pages; i++)
        res.append(dumbfmt({"<a href=\"/",board,"?pg=",std::to_string(i),"\">",std::to_string(i),"</a>"}));
    res.append(dumbfmt({"<br>Viewing page ",std::to_string(page),"."}));
    return res;
}