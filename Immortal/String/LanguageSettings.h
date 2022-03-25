#pragma once

#include "Core.h"
#include "Utils/json.h"

namespace Immortal
{

class WordsMap
{
public:
    WordsMap(const std::string &path)
    {
        auto json = JSON::Parse(path);

        auto &map = json["settings"]["map"];

        for (decltype(json)::iterator it = map.begin(); it != map.end(); ++it)
        {
            const auto &item = it->items();
            words[it.key()] = it.value().get<std::string>();
            int pause = 0;
        }
    }

    static const std::string &Get(const std::string &key)
    {
        const auto &it = That.words.find(key);
        return it == That.words.end() ? key : it->second;
    }

private:
    std::map<std::string, std::string> words;

    static WordsMap That;
};

}
