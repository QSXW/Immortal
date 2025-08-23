#include "Platform.h"
#import <Cocoa/Cocoa.h>

namespace Immortal
{

std::optional<String> FileDialogs::OpenFile(const char *filter)
{
    NSOpenPanel *op = [NSOpenPanel openPanel];
    if ([op runModal] == NSModalResponseOK) {
            NSURL *nsurl = [[op URLs] objectAtIndex:0];
            return std::string([[nsurl path] UTF8String]);
    }

    return std::nullopt;
}

std::optional<std::vector<String>> FileDialogs::OpenMultipleFiles(const char *filter)
{
    NSOpenPanel *op = [NSOpenPanel openPanel];
    [op setAllowsMultipleSelection:YES];
    if ([op runModal] == NSModalResponseOK) {
        NSArray *urls = [op URLs];
        std::vector<String> files;
        for (NSURL *url in urls) {
            files.push_back(String([[url path] UTF8String]));
        }
        return files;
    }

    return std::nullopt;
}

std::optional<std::string> FileDialogs::SaveFile(const char *filter)
{
    return std::nullopt;
}

}
