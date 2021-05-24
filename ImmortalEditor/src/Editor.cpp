#include "Immortal.h"
#include "EditorLayer.h"
#include "Immortal/Core/EntryPoint.h"


class Editor : public Immortal::Application
{
public:
	Editor()
		: Application({ UNICODE8("Immortal ±à¼­Æ÷"), 1920, 1080})
	{
		PushLayer(new Immortal::EditorLayer());
	}

	~Editor()
	{
		
	}

};

Immortal::Application* Immortal::CreateApplication()
{
	return new Editor();
}
