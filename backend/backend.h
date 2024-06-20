#pragma once 
#include <vector>
#include <string>


namespace be
{
    std::vector<std::string> get_banners(bool cache = true);
    std::string select_banner();
}