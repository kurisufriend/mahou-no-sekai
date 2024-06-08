#pragma once
#include <vector>
#include <string>

namespace botwall
{
    //returns answer || challenge filename
    std::pair<std::string, std::string> generate_captcha(unsigned int ipid, std::string secret);
    bool check_captcha(std::string ipid, std::string guess, std::string token, std::string secret);
};