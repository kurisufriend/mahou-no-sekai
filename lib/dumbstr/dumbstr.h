#pragma once
#include <string>
#include <vector>
#include <map>

#define foreach(a, b) for (auto b = a.begin(); b != a.end(); b++)
#define dfmt dumbfmt 

std::string dumbfmt(std::vector<std::string> o);
std::string dumbfmt_replace(std::string needle, std::string gold, std::string &haystack);
std::string dumbfmt_file(std::string path, std::map<std::string, std::string> dict, bool cach=true);
std::string dumbfmt_html_escape(std::string o);