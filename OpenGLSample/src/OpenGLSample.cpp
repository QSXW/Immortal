#include "Immortal.h"
#include "OpenGLSample2D.h"
#include "Framework/EntryPoint.h"

class ImageShowLayer : public Immortal::Layer
{
public:
	ImageShowLayer()
		: Layer("Example")
	{
		mVertexArray = Immortal::VertexArray::Create();

		float vertices[3 * 7] = {
			/* x, y, z */
			-0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
			 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			 0.0f,  0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f
		};

		uint32_t indices[3] = {
			0, 1, 2
		};

		Immortal::Ref<Immortal::VertexBuffer> mVertexBuffer = Immortal::VertexBuffer::Create(vertices, sizeof(vertices));
		Immortal::Ref<Immortal::IndexBuffer> mIndexBuffer = Immortal::IndexBuffer::Create(indices, 3);

		{
			Immortal::VertexLayout layout = {
				{ Immortal::ShaderDataType::Float3, "a_Position" },
				{ Immortal::ShaderDataType::Float4, "a_Color" }
			};

			mVertexBuffer->SetLayout(layout);
		}

		mVertexArray->AddVertexBuffer(mVertexBuffer);
		mVertexArray->SetIndexBuffer(mIndexBuffer);

		mSquareVertexArray = Immortal::VertexArray::Create();
		mTexture = Immortal::Texture2D::Create("D:/Digital Media/Assests/jpeg/lol1.jpg");

		float sqrv[5 * 4];
		memcpy(sqrv, squareVertices, 5 * 4 * sizeof(float));
		float *ptr = sqrv;
		float ratio = (float)mTexture->Width() / (float)mTexture->Height();
		for (int i = 0; i < 5 * 4; i += 5)
		{
			ptr[i] = ptr[i] * ratio;
		}

		Immortal::Ref<Immortal::VertexBuffer> squareVB = Immortal::VertexBuffer::Create(sqrv, sizeof(sqrv));
		{
			Immortal::VertexLayout layout = {
				{ Immortal::ShaderDataType::Float3, "a_Position" },
				{ Immortal::ShaderDataType::Float2, "a_TexCoord" }
			};

			squareVB->SetLayout(layout);
		}
		mSquareVertexArray->AddVertexBuffer(squareVB);

		uint32_t indices2[] = {
			0, 1, 2, 2, 3, 0
		};

		Immortal::Ref<Immortal::IndexBuffer> squareIndexBuffer = Immortal::IndexBuffer::Create(indices2, sizeof(indices2) / sizeof(uint32_t));
		mSquareVertexArray->SetIndexBuffer(squareIndexBuffer);

		shaderLib.Load("assets/shaders/Texture.glsl");
		shaderLib.Get("Texture")->Bind();
		shaderLib.Get("Texture")->SetUniform("u_Texture", 0);
	}

	void OnUpdate() override
	{
		//IM_TRACE("Delta time: {0}s ({1}ms)", Immortal::Application::Time.DeltaTime(), Immortal::Application::Time.Milliseconds());
		float deltaTime = Immortal::Application::App()->DeltaTime();
		mCamera.OnUpdate(deltaTime);

		Immortal::Renderer::BeginScene(mCamera.Camera());

		shaderLib.Get("Texture")->Bind();
		shaderLib.Get("Texture")->SetUniform("u_RGBA", mSquareColor);
		shaderLib.Get("Texture")->SetUniform("u_Luminance", mLuminance);
		mTexture->Bind();
		Immortal::Renderer::Submit(shaderLib.Get("Texture"), mSquareVertexArray);

		Immortal::Renderer::EndScene();
	}

	void OnGuiRender() override
	{
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
		static bool show_demo_window = false;
		float color = 0x0;

		static bool my_tool_active = true;

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu(IMMORTAL_CONSTANT_STRING_FILE))
			{
				if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_OPEN, "Ctrl+O"))
				{ 
					std::optional<std::string> filename = Immortal::FileDialogs::OpenFile(Immortal::FileDialogs::ImageFilter);
					if (filename.has_value())
					{
						mTexture = Immortal::Texture2D::Create(filename.value());
						float sqrv[5 * 4];
						memcpy(sqrv, squareVertices, 5 * 4 * sizeof(float));
						float *ptr = sqrv;
						float ratio = (float)mTexture->Width() / (float)mTexture->Height();
						for (int i = 0; i < 5 * 4; i += 5)
						{
							ptr[i] = ptr[i] * ratio;
						}

						Immortal::Ref<Immortal::VertexBuffer> squareVB = Immortal::VertexBuffer::Create(sqrv, sizeof(sqrv));
						{
							Immortal::VertexLayout layout = {
								{ Immortal::ShaderDataType::Float3, "a_Position" },
								{ Immortal::ShaderDataType::Float2, "a_TexCoord" }
							};

							squareVB->SetLayout(layout);
						}
						mSquareVertexArray->AddVertexBuffer(squareVB);

						uint32_t indices2[] = {
							0, 1, 2, 2, 3, 0
						};

						Immortal::Ref<Immortal::IndexBuffer> squareIndexBuffer = Immortal::IndexBuffer::Create(indices2, sizeof(indices2) / sizeof(uint32_t));
						mSquareVertexArray->SetIndexBuffer(squareIndexBuffer);
						mCamera.SetZoomLevel(1.1f);
						mCamera.Resize((float)Immortal::Application::App()->GetWindow().Width(), (float)Immortal::Application::App()->GetWindow().Height());
						mSquareColor = Immortal::Vector::Vector4(0);
						mLuminance = 0;
						mCamera.Camera().setPosition(Immortal::Vector::Vector3(0));
						mCamera.Camera().SetRotation(0);
					}

				}
				if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_SAVE, "Ctrl+S")) { /* Do stuff */ }
				if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_CLOSE, "Ctrl+W")) { my_tool_active = false; }
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu(IMMORTAL_CONSTANT_STRING_EDIT))
			{
				if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_OPEN, "Ctrl+O")) { /* Do stuff */ }
				if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_SAVE, "Ctrl+S")) { /* Do stuff */ }
				if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_CLOSE, "Ctrl+W")) { my_tool_active = false; }
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu(IMMORTAL_CONSTANT_STRING_VIEW))
			{
				if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_OPEN, "Ctrl+O")) { /* Do stuff */ }
				if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_SAVE, "Ctrl+S")) { /* Do stuff */ }
				if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_CLOSE, "Ctrl+W")) { my_tool_active = false; }
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu(IMMORTAL_CONSTANT_STRING_GRAPHICS))
			{
				if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_OPEN, "Ctrl+O")) { /* Do stuff */ }
				if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_SAVE, "Ctrl+S")) { /* Do stuff */ }
				if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_CLOSE, "Ctrl+W")) { my_tool_active = false; }
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu(IMMORTAL_CONSTANT_STRING_HELP))
			{
				if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_OPEN, "Ctrl+O")) { /* Do stuff */ }
				if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_SAVE, "Ctrl+S")) { /* Do stuff */ }
				if (ImGui::MenuItem(IMMORTAL_CONSTANT_STRING_CLOSE, "Ctrl+W")) { my_tool_active = false; }
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		if (show_demo_window)
		{
			ImGui::ShowDemoWindow(&show_demo_window);
		}
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin(IMMORTAL_CONSTANT_STRING_CONSOLE);
			ImGui::Checkbox(IMMORTAL_CONSTANT_STRING_DEMO_CONSOLE, &show_demo_window);
			//ImGui::ColorEdit4(UNICODE8("选择颜色"), Vector::value_ptr(mSquareColor) + 0);
			ImGui::SliderFloat(UNICODE8("红色"), &mSquareColor.r, -1, 1);
			ImGui::SliderFloat(UNICODE8("绿色"), &mSquareColor.g, -1, 1);
			ImGui::SliderFloat(UNICODE8("蓝色"), &mSquareColor.b, -1, 1);
			ImGui::SliderFloat(UNICODE8("透明度"), &mSquareColor.a, -1, 1);
			ImGui::SliderFloat(UNICODE8("亮度"), &mLuminance, -1, 1);
			ImGui::Text(IMMORTAL_CONSTANT_STRING_RENDER_RATE, 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}
	}

	void OnEvent(Immortal::Event &e) override
	{
		mCamera.OnEvent(e);
		Immortal::EventDispatcher dispatcher(e);
		dispatcher.Dispatch<Immortal::KeyPressedEvent>(IM_BIND_EVENT_FUNC(ImageShowLayer::OnKeyPressed));
	}

	bool OnKeyPressed(Immortal::KeyPressedEvent &e)
	{
		IM_CORE_INFO(e);

		return false;
	}

private:
	Immortal::Ref<Immortal::VertexArray> mVertexArray;
	Immortal::Ref<Immortal::VertexArray> mSquareVertexArray;

	uint32_t indices;

	Immortal::ShaderMap shaderLib;

	Immortal::OrthographicCameraController mCamera{ (float)Immortal::Application::App()->GetWindow().Width(), (float)Immortal::Application::App()->GetWindow().Height(), true };

	Immortal::Vector::Vector4 mSquareColor{ 0.0f, 0.0f, 0.0f, 0.0f };
	float mLuminance = 0.0f;

	Immortal::Ref<Immortal::Texture2D> mTexture;

	float squareVertices[5 * 4] = {
		/* x, y, z */
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f
	};
};

class OpenGLSample : public Immortal::Application
{
public:
	OpenGLSample()
	{
		// PushLayer(new ImageShowLayer());
		PushLayer(new OpenGLSample2D());
	}

	~OpenGLSample()
	{

	}
};

Immortal::Application* Immortal::CreateApplication()
{
	return new OpenGLSample();
}
