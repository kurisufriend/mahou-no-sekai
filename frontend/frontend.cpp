#include "frontend.h"
#include "../backend/backend.h"
#include "../lib/dumbstr/dumbstr.h"
#include "../lib/tinternet/tinternet.h"
#include <string>
#include <vector>

std::string generate_backlinks(std::string body, std::string board, std::string op)
{
    const std::vector<char> numerics = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    std::string::size_type first = std::string::npos;
    first = body.find("&gt;&gt;");
    body = dumbfmt_replace("\r", "", body); // just in case
    while (first != std::string::npos)
    {
        int end = first+8; // skip past the 8chars of escaped `>>'
        while (ezin(body.at(end), numerics) && end < body.length())
            end++;
        std::string no = body.substr(first+8, end-(first+8));
        std::string href = "/"+board+"/thread/"+op+"#"+no;
        dumbfmt_replace("&gt;&gt;"+no, "<a href=\""+href+"\" class=\"backlink\">&gayt;&gayt;"+no+"</a>", body);
        first = body.find("&gt;&gt;");
    }
    dumbfmt_replace("&gayt;&gayt;", "&gt;&gt;", body);
    return body;
}

std::string fe::generate_board(sqlite3* db, std::string name, std::map<std::string, std::string> &qstring, mns::gcache* gcache, int threadid)
{
    if (gcache->board_info_cache.find(name) == gcache->board_info_cache.end())
        gcache->update_board_info(db, name);
    row board = gcache->board_info_cache.at(name);

    std::vector<std::string> boards = be::get_boards(db);
    std::vector<std::string> links;
    foreach(boards, i)
        {links.push_back(dumbfmt({"<a href='/",*i,"'>",*i,"</a>"}));}

    return dumbfmt_file("./static/board.html", {
        {
            {"boardname", name},
            {"boardtopic", board["topic"]},
            {"boardflavor", board["flavor"]},
            {"banner source", dumbfmt({"/banners/", be::select_banner()})},
            {"top board list", dumbfmt({"[",dumbfmt(links, " / "),"]"})},
            {"threads", threadid == -1 ? fe::generate_index(db, name, qstring, gcache, false) : fe::generate_thread(db, name, threadid, gcache)},
            {"form subject visibility", threadid == -1 ? "visible" : "collapse"},
            {"reply visibility", threadid != -1 ? "visible" : "collapse"},
            {"threadid", std::to_string(threadid)},
            {"captcha_token", "lol do this later"},
            {"challenge", "/media/kurisuqt43.jpg"}, //TODO wire up captcha
            {"default name", "Anonymous"} //TODO make board option for default name
        }
    });
}

std::string fe::generate_index(sqlite3 *db, std::string board, std::map<std::string, std::string> &qstring, mns::gcache* gcache, bool catalog)
{
    int page = 1;
    if (qstring.find("pg") != qstring.end())
        page = sstoi(qstring["pg"], 1);
    if (gcache->board_cache.find(board) == gcache->board_cache.end())
        gcache->update_board(db, board);
    rows threads = gcache->board_cache.at(board);
    if (threads.size() > 10)
        threads = rows(threads.begin(), threads.begin()+9);

    int count = threads.size();
    int pages = int(count/10)+1;
    std::string res = dumbfmt({std::to_string(count), " thread(s) currently on-board, ", std::to_string(pages), " page(s).", "<hr>"});

    static std::map<std::string, rows> cach;

    foreach (threads, i)
    {
        int op = sstoi((*i)["no"]);
        if (gcache->index_cache.find(std::pair<std::string, int>(board, op)) == gcache->index_cache.end())
            gcache->update_thread_index(db, board, op);
        rows posts = gcache->index_cache.at(std::pair<std::string, int>(board, op)).second;
        if (posts.empty())
            return "err: no posts objects found for known thread object "+(*i)["no"]+"<br>";
        int replies = posts.size();
        foreach(posts, pos)
        {
            row post = *pos;
            time_t ti = (time_t)sstoi(post["time"], 0);
            bool is_op = post["no"] == post["op"];
            res.append(
                dumbfmt_file("./static/template/post.html", {
                    {"no", post["no"]},
                    {"subject", is_op ? (*i)["subject"] : ""},
                    {"name", post["name"]},
                    {"trip", post["trip"]},
                    {"posterid", ""},
                    {"date", ctime(&ti)},
                    {"body", generate_backlinks(post["body"], post["board"], post["op"])},
                    {"attrs", is_op ? "" : "replypost"},
                    {"replylink", is_op ? dumbfmt({"<a href='/",board,"/thread/",post["op"],"'>[Reply]</a> ",(*i)["replies"], " replies."}) : ""},
                    {"image", post["filename"] == "" ? "" : dumbfmt_file("./static/template/image.html", {
                        {"filename", post["filename"]},
                        {"uploadname", post["uploadname"]},
                        {"thumbname", post["filename"]},
                        {"size", is_op ? "250" : "125"}
                    })}
                })
            );

            if (pos == posts.begin() && replies > 4)
            {
                res.append(dumbfmt({std::to_string(replies - 4)," replies hidden. <a href='/",board,"/thread/",post["op"],"'>[click here to open full thread]</a>"}));
                pos = posts.end() - 4;
            }
        }
        res.append("<hr>");
    }
    res.append("Pages: ");
    for (int i = 1; i <= pages; i++)
        res.append(dumbfmt({"<a href=\"/",board,"?pg=",std::to_string(i),"\">",std::to_string(i),"</a>"}));
    res.append(dumbfmt({"<br>Viewing page ",std::to_string(page),"."}));
    return dumbfmt_file("./static/template/tview.html", {
        {"temp", res}
    });
}

std::string fe::generate_thread(sqlite3* db, std::string board, int op, mns::gcache* gcache)
{
    if (gcache->index_cache.find(std::pair<std::string, int>(board, op)) == gcache->index_cache.end())
        gcache->update_thread_index(db, board, op);
    auto t = gcache->index_cache.at(std::pair<std::string, int>(board, op));
    row thread = t.first;
    rows posts = t.second;
    if (thread.empty())
        return "no such thread :(";
    if (posts.empty())
        return "posts missing~!";
    int replies = sstoi(thread["replies"]);

    std::string res;
    res.append(dumbfmt({"<a href='/",board,"'>[Return to board]</a><hr>"}));
    foreach(posts, pos)
    {
        row post = *pos;
        time_t ti = (time_t)std::stoi(post["time"]);
        bool is_op = post["no"] == post["op"];
        res.append(
            dumbfmt_file("./static/template/post.html", {
                {"no", post["no"]},
                {"subject", is_op ? thread["subject"] : ""},
                {"name", post["name"]},
                {"trip", post["trip"]},
                {"posterid", ""},
                {"date", ctime(&ti)},
                {"body", generate_backlinks(post["body"], post["board"], post["op"])},
                {"attrs", is_op ? "" : "replypost"},
                {"replylink", ""},
                {"image", post["filename"] == "" ? "" : dumbfmt_file("./static/template/image.html", {
                    {"filename", post["filename"]},
                    {"uploadname", post["uploadname"]},
                    {"thumbname", post["filename"]},
                    {"size", is_op ? "250" : "125"}
                })}
            })
        );
    }
    return dumbfmt_file("./static/template/tview.html", {
        {"temp", res}
    });
}