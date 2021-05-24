#pragma once

#if defined( IMMORTAL_PLATFORM_WINDOWS )
 
extern Immortal::Application* Immortal::CreateApplication();

#include <memory>

int main(int argc, char **argv)
{
	Immortal::Log::Init();
	auto app = Immortal::CreateApplication();
	app->Run();
	delete app;

	return 0;
}

#endif