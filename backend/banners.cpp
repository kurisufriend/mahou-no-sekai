#include "backend.h"
#include "../lib/tinydir/tinydir.h"
#include "../lib/women/women.h"

#include <string>
#include <iostream>



std::vector<std::string> be::get_banners(bool cache)
{
    static std::vector<std::string> res;

    if (cache && res.size() > 0)
        {return res;}
    res.clear();
    tinydir_dir d;
    tinydir_open(&d, "./banners");
    while(d.has_next)
    {
        tinydir_file f;
        tinydir_readfile(&d, &f);
        if (!f.is_dir)
            {res.push_back(f.name);}
        tinydir_next(&d);
    }

    return res;
}

std::string be::select_banner()
{
    auto bs = be::get_banners();
    if (bs.empty())
        {return "?.png";}
    return choice_vector<std::string>(bs);
}
