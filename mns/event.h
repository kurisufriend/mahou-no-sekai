#pragma once
#include <string>

namespace mns
{
    struct event
    {
        float timestamp;
        std::string expel(mns::event event);
    };
}