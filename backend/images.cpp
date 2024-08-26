#include "backend.h"
#include "../lib/dumbstr/dumbstr.h"
#include "../lib/tinternet/tinternet.h"
#include <ios>
#include <iostream>
#include <fstream>
#include <cstdio>

be::err be::upload_image(std::string filename, std::string data, std::string basepath)
{
    //TODO thumbnailing
    if (filename == "")
        return err(2, "");
    std::string ext = dumbfmt_split(filename, '.').back();
    std::vector<std::string> allowed = {"png", "jpg", "gif"};
    if (!ezin(ext, allowed))
        return err(0, "file should be of type "+dumbfmt(allowed, ","));
    //TODO max upload size as board setting
    if (data.length() > 4 * 1000 * 1000)
        return err(-1, "file too big! should be <= 4MB");
    if (filename.length() > 30)
        return err(-2, "filename too long");
    std::string local_fn = std::to_string(time(0))+"."+ext;
    std::ofstream f;
    f.open(basepath+local_fn, std::ios_base::binary);
    f << data;
    f.close();
    return err(1, local_fn);
}