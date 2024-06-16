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
            words[it.key()] = { it.value().get<std::string>(), StringEncoding::UTF8 };
        }
    }

    static const String &Get(const String &key)
    {
        const auto &it = That.words.find(key);
        return it == That.words.end() ? key : it->second;
    }

    static const std::map<String, String> GetWords()
    {
		return That.words;
    }

private:
    std::map<String, String> words;

    static WordsMap That;
};

}
