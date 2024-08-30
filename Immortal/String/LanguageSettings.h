#pragma once

#include "Core.h"
#include "Helper/json.h"
#include "IString.h"
#include <map>

namespace Immortal
{

class Translator
{
public:
	Translator(const String &path) :
	    words{}
    {
		AddLanguageSettingsImpl(path);
    }

    const String &TranslateImpl(const String &key)
    {
		const auto &it = words.find(key);
        if (it == words.end())
        {
			words[key] = key;
			return words[key];
        }     

        return it->second;
    }

protected:
    void AddLanguageSettingsImpl(const String &path)
    {
        try
        {
			auto json = JSON::Parse(path);
			auto &map = json["settings"]["map"];

			for (decltype(json)::iterator it = map.begin(); it != map.end(); ++it)
			{
				const auto &item = it->items();
				words[it.key()] = {it.value().get<std::string>(), StringEncoding::UTF8};
			}
        }
        catch (const std::exception &e)
        {
			LOG::ERR("Failed to load language settings - `{}`", path.c_str());
        }
    }

public:
    static const String &Translate(const String &key)
    {
		return This.TranslateImpl(key);
    }

    static void AddLanguageSettings(const String &path)
    {
		This.AddLanguageSettingsImpl(path);
    }

    static const std::map<String, String> GetWords()
    {
		return This.words;
    }

protected:
    std::map<String, String> words;

    static Translator This;
};

}
