#pragma once
#include "Immortal.h"

class OpenGLSample2D : public Immortal::Layer
{
public:
	OpenGLSample2D();
	virtual ~OpenGLSample2D() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	void OnUpdate() override;
	virtual void OnGuiRender() override;
	void OnEvent(Immortal::Event& e) override;



private:
	Immortal::OrthographicCameraController mCameraController;

	Immortal::Ref<Immortal::VertexArray> mSquareVA;
	Immortal::Ref<Immortal::Shader> mFlatColorShader;
	Immortal::Ref<Immortal::Texture2D> mTexture;

	float mLuminance{ 0 };
	Immortal::Vector::Vector4 mSquareColor { 0.0f, 0.0f, 0.0f, 0.0f };
};
