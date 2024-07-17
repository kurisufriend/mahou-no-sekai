#include "backend.h"
#include "../lib/dumbstr/dumbstr.h"

std::map<std::string, std::string> be::getquery(std::string raw)
{
    std::map<std::string, std::string> res;

    if(raw.substr(0, 1) != "?")
        return res;
    raw = dumbfmt_replace("?", "&", raw);
    std::vector<std::string> items = dumbfmt_split(raw, '&');
    foreach(items, i)
    {
        if (*i == "")
            continue;
        std::vector<std::string> sides = dumbfmt_split(*i, '=');
        res.emplace(sides.at(0), sides.size() > 1 ? sides.at(1) : "");
    }
    return res;
}