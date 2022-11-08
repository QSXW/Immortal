/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include "Core.h"
#include <string>
#include <unordered_map>

namespace Immortal
{

class Arguments
{
public:
    Arguments(int argc, char **argv);

    const std::string &operator[](const std::string &key) const;

protected:
    std::unordered_map<std::string, std::string> args;

};

}
