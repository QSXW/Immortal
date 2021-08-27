#include "impch.h"
#include "OrthographicCameraController.h"

#include "Framework/Application.h"

namespace Immortal {

	OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool rotation)
		: mAspectRatio(aspectRatio), mCamera(-mAspectRatio * mZoomLevel, mAspectRatio * mZoomLevel, -mZoomLevel, mZoomLevel), mRotation(rotation)
	{

	}

	OrthographicCameraController::OrthographicCameraController(float width, float height, bool rotation)
		: mAspectRatio(width / height), mCamera(-mAspectRatio * mZoomLevel, mAspectRatio * mZoomLevel, -mZoomLevel, mZoomLevel), mRotation(rotation)
	{
	
	}

	void OrthographicCameraController::OnUpdate(Timestep ts)
	{
		if (Input::IsKeyPressed(KeyCode::A) /*|| Input::GetMouseX() < 50.0f*/)
		{
			mCameraPosition.x -= cos(Vector::Radians(mCameraRotation)) * mCameraTranslationSpeed * ts;
			mCameraPosition.y -= sin(Vector::Radians(mCameraRotation)) * mCameraTranslationSpeed * ts;
		}
		else if (Input::IsKeyPressed(KeyCode::D) /*|| Input::GetMouseX() > Application::Get().GetWindow().Width() - 50.0f*/)
		{
			mCameraPosition.x += cos(Vector::Radians(mCameraRotation)) * mCameraTranslationSpeed * ts;
			mCameraPosition.y += sin(Vector::Radians(mCameraRotation)) * mCameraTranslationSpeed * ts;
		}

		if (Input::IsKeyPressed(KeyCode::W) /*|| Input::GetMouseY() < 50.0f*/)
		{
			mCameraPosition.x += -sin(Vector::Radians(mCameraRotation)) * mCameraTranslationSpeed * ts;
			mCameraPosition.y += cos(Vector::Radians(mCameraRotation)) * mCameraTranslationSpeed * ts;
		}
		else if (Input::IsKeyPressed(KeyCode::S) /*|| Input::GetMouseY() > Application::Get().GetWindow().Height() - 50.0f*/)
		{
			mCameraPosition.x -= -sin(Vector::Radians(mCameraRotation)) * mCameraTranslationSpeed * ts;
			mCameraPosition.y -= cos(Vector::Radians(mCameraRotation)) * mCameraTranslationSpeed * ts;
		}

		if (mRotation)
		{
			if (Input::IsKeyPressed(KeyCode::Q))
				mCameraRotation += mCameraRotationSpeed * ts;
			if (Input::IsKeyPressed(KeyCode::E))
				mCameraRotation -= mCameraRotationSpeed * ts;

			if (mCameraRotation > 180.0f)
				mCameraRotation -= 360.0f;
			else if (mCameraRotation <= -180.0f)
				mCameraRotation += 360.0f;

			mCamera.SetRotation(mCameraRotation);
		}

		mCamera.setPosition(mCameraPosition);
		mCameraTranslationSpeed = mZoomLevel;
	}

	void OrthographicCameraController::OnEvent(Event & e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(std::bind(&OrthographicCameraController::OnMouseScrolled, this, std::placeholders::_1));
		dispatcher.Dispatch<WindowResizeEvent>(std::bind(&OrthographicCameraController::OnWindowResized, this, std::placeholders::_1));
	}

	void OrthographicCameraController::Reset(Vector::Vector3 &position, float rotation)
	{
		mCameraPosition = position;
		mCameraRotation = rotation;
		mCamera.Set(mCameraPosition, mCameraRotation);
	}

	void OrthographicCameraController::Resize(float width, float height)
	{
		mAspectRatio = width / height;
		mCamera.SetProjection(-mAspectRatio * mZoomLevel, mAspectRatio * mZoomLevel, -mZoomLevel, mZoomLevel);
	}

	bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent & e)
	{
		mZoomLevel -= e.GetYOffset() * 0.25f;
		mZoomLevel = std::max(mZoomLevel, 0.1f);
		mCamera.SetProjection(-mAspectRatio * mZoomLevel, mAspectRatio * mZoomLevel, -mZoomLevel, mZoomLevel);
		return false;
	}

	bool OrthographicCameraController::OnWindowResized(WindowResizeEvent & e)
	{
		Resize((float)e.Width(), (float)e.Height());
		return false;
	}

}
