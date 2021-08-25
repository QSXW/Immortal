#pragma once

#if defined( IMMORTAL_PLATFORM_WINDOWS )
#include "ImmortalCore.h"
extern Immortal::Application* Immortal::CreateApplication();

#include <memory>
#include <cstdlib>

int main(int argc, char **argv)
{
	Immortal::Utils::SafeChunk = new UINT8[1024];

	system("chcp 65001 & cls");
	Immortal::LOG::Init();
	auto app = Immortal::CreateApplication();
	app->Run();
	delete app;

	delete[] Immortal::Utils::SafeChunk;
	return 0;
}

#endif