#pragma once

#include "nlohmann_json.h"

#include <fstream>

namespace Immortal
{

class JSON
{
public:
    using SuperJSON = nlohmann::json;

public:
    static SuperJSON Parse(const std::string &path)
    {
        std::ifstream input{ path, std::ifstream::in };
        
        if (!input.is_open())
        {
            return SuperJSON{};
        }
        SuperJSON json;
        input >> json;

        return json;
    }

};

}
