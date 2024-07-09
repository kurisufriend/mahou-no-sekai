#include "lib/mongoose/mongoose.h"
#include "lib/sqlite/sqlite3.h"
#include "lib/dumbstr/dumbstr.h"
#include "lib/json.hpp"
#include "lib/tinternet/tinternet.h"

#include "backend/backend.h"
#include "frontend/frontend.h"

#include <cctype>
#include <cstdlib>
#include <iostream>
#include <map>
#include <any>
#include <string>
#include <vector>
#include <fstream>

#define XCLACKSOVERHEAD "X-Clacks-Overhead: GNU Terry Pratchett, GNU Aaron Swartz, GNU Hal Finney, GNU Norm Macdonald, GNU Gilbert Gottfried, GNU Aniki, GNU Terry Davis, GNU jstark, GNU John McAfee, GNU asshurtmacfags\n"
#define THROW404() mg_http_reply(c, 404, headers.c_str(), "the name's huwer, as in who are the fuck is you? 404")


typedef mg_connection connection;
typedef mg_http_message message;

sqlite3* db;

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

        std::vector<std::string> boards = be::get_boards(db);

        if(mg_http_match_uri(msg, "/"))
        {
            headers.append("Content-Type: text/html;charset=utf-8\n");
            mg_http_reply(c, 200, headers.c_str(),
                dumbfmt_file("./static/index.html", {
                    {"body", dumbfmt({"oha~! "})}
                }).c_str());
        }
        else if(ezin(url.substr(1), boards))
        {
            headers.append("Content-Type: text/html;charset=utf-8\n");
            mg_http_reply(c, 200, headers.c_str(), fe::generate_board(db, url.substr(1)).c_str());
        }
        else if(mg_http_match_uri(msg, "/static/*") || (mg_http_match_uri(msg, "/banners/*")))
        {
            mg_http_serve_opts opts = {.root_dir = "."};
            mg_http_serve_dir(c, msg, &opts);       
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


    if (!inited)
    {
        be::init_database_schemae(db);
        be::make_board(db, cfg["board"], cfg["board_topic"], cfg["board_flavor"], 0);
        //TESTING
        be::make_board(db, "vt", "Virginia Tech", "Go Hokies!", 0);
    }

    mg_http_listen(&mongoose, cfg["host"].get<std::string>().c_str(), callback, &mongoose);
    while (true) {mg_mgr_poll(&mongoose, 1000);}
    sqlite3_close(db);

    return 0;
}