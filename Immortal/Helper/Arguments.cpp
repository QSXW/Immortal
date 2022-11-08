#include "Arguments.h"
#include "Framework/Log.h"

namespace Immortal
{

static std::string invalid;

Arguments::Arguments(int argc, char **argv)
{
    char buffer[255] = {};
    for (int i = 1; i < argc; i++)
    {
        auto arg = argv[i];
        if (arg[0] == '-' || arg[1] == '-')
        {
            arg += 2;
            for (int j = 0; *arg && j < SL_ARRAY_LENGTH(buffer); j++)
            {
                if (*arg == '=')
                {
                    std::string key;
                    std::string value;
                    arg++;
                    buffer[j++] = '\0';
                    key   = buffer;
                    value = arg;
                    args[key] = std::move(value);
                    break;
                }
                buffer[j] = *arg++;
            }
        }
        else
        {
            LOG::ERR("Unrecognized options::{}", arg);
        }
    }
}

const std::string &Arguments::operator[](const std::string &key) const
{
    auto it = args.find(key);
    return it != args.end() ? it->second : invalid;
}

}
