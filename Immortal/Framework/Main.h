#pragma once

#ifdef WINDOWS
#include "ImmortalCore.h"

extern Immortal::Application* Immortal::CreateApplication();

#include <memory>
#include <cstdlib>

int main(int argc, char **argv)
{
    system("chcp 65001 & cls");
    Immortal::LOG::Setup();

    std::unique_ptr<Immortal::Application> app{ nullptr };
    app.reset(Immortal::CreateApplication());

    app->Run();

    return 0;
}

#endif
