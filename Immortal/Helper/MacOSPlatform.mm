#include "Platform.h"
#import <Cocoa/Cocoa.h>

namespace Immortal
{

std::optional<std::string> FileDialogs::OpenFile(const char *filter)
{
    NSOpenPanel *op = [NSOpenPanel openPanel];
    if ([op runModal] == NSModalResponseOK) {
            NSURL *nsurl = [[op URLs] objectAtIndex:0];
            return std::string([[nsurl path] UTF8String]);
    }

    return std::nullopt;
}

std::optional<std::string> FileDialogs::SaveFile(const char *filter)
{
    return std::nullopt;
}

}
