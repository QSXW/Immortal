#pragma once

#include "Core.h"
#include "Helper/json.h"
#include "IString.h"

namespace Immortal
{

class WordsMap
{
public:
    WordsMap(const String &path)
    {
        auto json = JSON::Parse(path);

        auto &map = json["settings"]["map"];

        for (decltype(json)::iterator it = map.begin(); it != map.end(); ++it)
        {
            const auto &item = it->items();
            words[it.key()] = { it.value().get<std::string>(), String::Type::Unicode8 };
        }
    }

    static const String &Get(const String &key)
    {
        const auto &it = That.words.find(key);
        return it == That.words.end() ? key : it->second;
    }

private:
    std::map<String, String> words;

    static WordsMap That;
};

}
