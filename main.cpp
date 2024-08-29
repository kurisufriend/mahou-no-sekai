#include "lib/mongoose/mongoose.h"
#include "lib/sqlite/sqlite3.h"
#include "lib/dumbstr/dumbstr.h"
#include "lib/json.hpp"
#include "lib/tinternet/tinternet.h"
#include "lib/niphash.h"
#include "lib/women/women.h"

#include "backend/backend.h"
#include "frontend/frontend.h"
#include "mns/cache.h"
#include "mns/event.h"

#include <cctype>
#include <cstdlib>
#include <iostream>
#include <map>
#include <any>
#include <string>
#include <vector>
#include <fstream>


#define XCLACKSOVERHEAD "X-Clacks-Overhead: GNU Terry Pratchett, GNU Aaron Swartz, GNU Hal Finney, GNU Norm Macdonald, GNU Gilbert Gottfried, GNU Aniki, GNU Terry Davis, GNU jstark, GNU John McAfee, GNU asshurtmacfags, GNU vince\n"
#define THROW404() mg_http_reply(c, 404, headers.c_str(), "the name's huwer, as in who are the fuck is you? 404")
#define BUFFETCHHTTPVAR(m, key) char buffer_##key[10000]; \
mg_http_get_var(&(m->body), #key, buffer_##key, sizeof(buffer_##key)); \
const char* key = buffer_##key;

typedef mg_connection connection;
typedef mg_http_message message;

sqlite3* db;
mns::evmanager eman;
mns::gcache gc;


bool mg_match_boards(std::vector<std::string> &boards, message* msg, bool thread = false)
{
    foreach(boards, i)
    {
        if (mg_http_match_uri(msg, dumbfmt({"/",*i,"/"}).c_str()) ||
            mg_http_match_uri(msg, dumbfmt({"/",*i,!thread ? "" : "/thread/*"}).c_str()))
            return true;
    }
    return false;
}

void callback(connection* c, int ev, void* ev_data, void* fn_data)
{
    std::fstream f;
    f.open("./config.json");
    nlohmann::json cfg = nlohmann::json::parse(f);
    f.close();

    if (ev == MG_EV_HTTP_MSG)
    {
        std::string headers = XCLACKSOVERHEAD;
        message* msg = (message*)ev_data;
        std::string url = msg->uri.len != 0 ? msg->uri.ptr : "/";
        url = dumbfmt_before(url, ' ');
        std::map<std::string, std::string> GET = be::getquery(dumbfmt_after(url, '?'));
        url = dumbfmt_before(url, '?');

        std::vector<std::string> boards = be::get_boards(db);

        if(mg_http_match_uri(msg, "/"))
        {
            headers.append("Content-Type: text/html;charset=utf-8\n");
            mg_http_reply(c, 200, headers.c_str(),
                dumbfmt_file("./static/index.html", {
                    {"body", dumbfmt({"oha~! "})}
                }).c_str());
        }
        else if(mg_match_boards(boards, msg))
        {
            headers.append("Content-Type: text/html;charset=utf-8\n");
            mg_http_reply(c, 200, headers.c_str(), fe::generate_board(db, dumbfmt_replace("/", "", url), GET, &gc).c_str());
        }
        else if(mg_match_boards(boards, msg, true))
        {
            headers.append("Content-Type: text/html;charset=utf-8\n");
            std::vector<std::string> parts = dumbfmt_split(url, '/');
            int threadid = sstoi(parts.at(3));
            if (threadid < 0)
            {
                THROW404();
                return;
            }
            mg_http_reply(c, 200, headers.c_str(), fe::generate_board(db, parts.at(1), GET, &gc, threadid).c_str());
        }
        else if(mg_http_match_uri(msg, "/static/*") || 
        (mg_http_match_uri(msg, "/banners/*")) ||
        (mg_http_match_uri(msg, "/media/*")) ||
        (mg_http_match_uri(msg, "/thumbs/*")))
        {
            mg_http_serve_opts opts = {.root_dir = "."};
            mg_http_serve_dir(c, msg, &opts);       
        }
        else if (mg_http_match_uri(msg, "/post"))
        {
            mg_http_part part;
            size_t remaining = 0;
            std::map<std::string, std::pair<std::string, std::string>> data;
            while ((remaining = mg_http_next_multipart(msg->body, remaining, &part)) > 0) {
                std::string name = std::string(part.name.ptr, part.name.len);
                std::string filename = std::string(part.filename.ptr, part.filename.len);
                std::string body = std::string(part.body.ptr, part.body.len);
                data[name] = std::pair<std::string, std::string>(filename, body);
            }

            be::err ier = be::upload_image(data["filename"].first, data["filename"].second);

            be::err er;
            if (ier.first > 0)
            {
                er = be::handle_post_attempt(db, &eman,
                    data["boardname"].second,
                    data["threadid"].second, 
                    data["postsubject"].second, 
                    data["postname"].second, 
                    data["postbody"].second, 
                    ier.second,
                    data["filename"].first, 
                    data["token"].second, 
                    data["guess"].second,
                    data["sage"].second,
                    data["flags"].second,
                    data["asnlock"].second
                );
            }

            std::string resp = dumbfmt_file("./static/post.html", {
                {"image res", (ier.second.length() < 1 || ier.second.at(0) == ' ' ? "" : "uploaded ") + ier.second},
                {"post res", (er.second == "" && (ier.second.length() >= 1 && ier.second.at(0) != ' ' )) ? "post succeeded!" : er.second},
                {"board", data["boardname"].second},
                {"thread", (data["threadid"].second != "-1") ? ("/thread/"+data["threadid"].second) : ""}
            });
            if (ier.first > 0 && er.first > 0)
                {mg_http_reply(c, 200, "", resp.c_str());}
            else
                {mg_http_reply(c, 400, "", resp.c_str());}


        }
#define gimmick(inp, out)else if (mg_http_match_uri(msg, inp)) {mg_http_reply(c, 418, headers.c_str(), out);}
        gimmick("/rose", "won")
        else 
        {
            THROW404();
        }
    }

}

int main(int argc, char* argv[])
{
    mg_mgr mongoose;
    mg_mgr_init(&mongoose);

    bool inited = true;
    if (access("./the.db", F_OK) == -1)
        inited = false;

    if (sqlite3_open("./the.db", &db))
    {
        std::cout << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return -1;
    }
    
    std::fstream f;
    f.open("./config.json");
    nlohmann::json cfg = nlohmann::json::parse(f);
    f.close();

    gc = mns::gcache{};

    eman.db = db;
    eman.gc = &gc;
    eman.cfg = cfg;

    if (!inited)
    {
        be::init_database_schemae(db);
        be::make_board(db, cfg["board"], cfg["board_topic"], cfg["board_flavor"], 0);
        //TESTING
        be::make_board(db, "vt", "Virginia Tech", "Go Hokies!", 0);
    }

    eman.load("./log_path");

    mg_http_listen(&mongoose, cfg["host"].get<std::string>().c_str(), callback, &mongoose);
    while (true) {mg_mgr_poll(&mongoose, 1000);}
    sqlite3_close(db);

    return 0;
}