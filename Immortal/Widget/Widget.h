#pragma once

#include <string>

namespace Widget
{

class Super
{
public:
    Super() :
        text{ "Undefined" }
    {

    }

    Super(const std::string &name) :
        text{ name }
    {

    }

public:
    std::string text;
};

}
