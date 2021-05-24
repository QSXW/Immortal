#pragma once

#include "ImmortalCore.h"
#include "Immortal/Events/MouseEvent.h"
#include "Immortal/Render/Camera.h"

namespace Immortal {

	class EditorCamera : public Camera
	{
	public:
		EditorCamera() = default;
		EditorCamera(const Vector::Matrix4 &projection);

		void Focus(const Vector::Vector3 &focusPoint);
		void OnUpdate();
		void OnEvent(Event &e);

		float &Distance() { return mDistance; }

		void SetViewportSize(const Vector::Vector2 &size);

		Vector::Vector3 UpDirection();
		Vector::Vector3 RightDirection();
		Vector::Vector3 ForwardDirection();

		const Vector::Vector3 &Position() const { return mPosition; }
		Vector::Quaternion Orientation() const;

		float Pitch() const { return mPitch; }
		float Yaw() const { return mYaw; }

	private:
		void UpdateView();
		bool OnMouseScroll(MouseScrolledEvent &e);

		void MousePan(const Vector::Vector2 &delta);
		void MouseRotate(const Vector::Vector2 &delta);
		void MouseZoom(float delta);

		Vector::Vector3 CalculatePosition();

		Vector::Vector2 PanSpeed() const;
		constexpr float RotateSpeed()	 const;
		float ZoomSpeed() const;

	private:

		Vector::Vector3 mPosition{ 0.0f, 0.0f, 0.0f };
		Vector::Vector3 mRotation{ 0.0f, 0.0f, 0.0f };
		Vector::Vector3 mFocalPoint{ 0.0f, 0.0f, 0.0f };

		Vector::Vector2 mInitialPosition{ 0.0f, 0.0f };
		Vector::Vector3 mInitialFocalPoint{ 0.0f, 0.0f, 0.0f };
		Vector::Vector3 mInitialRotation{ 0.0f, 0.0f, 0.0f };

		Vector::Vector2 mViewportSize{ 1280.0f, 720.0f };

		float FOV = 45.0f;

		float mDistance{ 8.0f };
		float mPitch{ 0.0f }; /* x axis */
		float mYaw{ 0.0f }; /* y axis */
		float mMinFocusDistance{ 100.0f };
		bool mPanning{ true };
		bool mRotating{ true };
	};

}

