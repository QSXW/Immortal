#include "Immortal.h"
#include "EditorLayer.h"
#include "Framework/Main.h"


class Editor : public Immortal::Application
{
public:
	Editor()
		: Application({ U8("Immortal ±à¼­Æ÷"), 1920, 1080})
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
