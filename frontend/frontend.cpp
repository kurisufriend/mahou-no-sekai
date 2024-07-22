#include "frontend.h"
#include "../backend/backend.h"
#include "../lib/dumbstr/dumbstr.h"
#include <string>
#include <vector>

std::string fe::generate_board(sqlite3* db, std::string name, std::map<std::string, std::string> &qstring, int threadid, bool docache)
{
    static std::map<std::string, row> cache;

    row board;
    std::string res;

    if (docache && cache.find(name) != cache.end())
    {
        board = cache.at(name);
    }
    else
    {
        rows r = sqleasy_q{db, dumbfmt({"select * from boards where name=\"", name, "\" limit 1;"})}.exec();
        board = r.at(0);
        cache.emplace(name, board);
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
            {"threads", fe::generate_index(db, name, qstring, docache)}
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

    static std::map<std::string, rows> cach;

    foreach (r, i)
    {
        rows post_;
        if (cache && cach.find((*i)["no"]) != cach.end())
        {
            post_ = cach.at((*i)["no"]);
        }
        else
        {
            rows poasts = sqleasy_q{db, dumbfmt({"select * from posts where op=\"",(*i)["no"],"\""})}.exec();
            cach.emplace((*i)["no"], poasts);
            post_ = poasts;
        }
        if (post_.empty())
            return "";
        int replies = post_.size();
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
                    {"body", post["body"]},
                    {"attrs", is_op ? "" : "replypost"},
                    {"replylink", is_op ? dumbfmt({"<a href='/",board,"/thread/",post["op"],"'>[Reply]</a>"}) : ""},
                    {"image", post["filename"] == "" ? "" : dumbfmt_file("./static/template/image.html", {
                        {"filename", post["filename"]},
                        {"uploadname", post["uploadname"]},
                        {"thumbname", post["filename"]},
                        {"size", is_op ? "250" : "125"}
                    })}
                })
            );

            if (pos == post_.begin() && replies > 4)
            {
                res.append(dumbfmt({std::to_string(replies - 4)," replies hidden. <a href='/",board,"/thread/",post["op"],"'>[click here to open full thread]</a>"}));
                pos = post_.end() - 4;
            }
        }
        res.append("<hr>");
    }
    res.append("Pages: ");
    for (int i = 1; i <= pages; i++)
        res.append(dumbfmt({"<a href=\"/",board,"?pg=",std::to_string(i),"\">",std::to_string(i),"</a>"}));
    res.append(dumbfmt({"<br>Viewing page ",std::to_string(page),"."}));
    return res;
}

std::string fe::generate_thread(sqlite3* db, std::string board, int op)
{
    return "hi :)";
}