#pragma once

#include "Core.h"

#include "SceneCamera.h"
namespace Immortal
{
	class ObserverCamera : public SceneCamera
	{
	public:
		enum class Type : INT32
		{
			LookAt,
			FirstPerson
		};

	public:
		ObserverCamera() : SceneCamera()
		{

		}
		void OnUpdate(float deltaTime);

	private:
		void Update(float deltaTime);
		bool UpdateGamepad(Vector::Vector2 axisLeft, Vector::Vector2 axisRight, float deltaTime);
	private:
		ObserverCamera::Type mType = ObserverCamera::Type::FirstPerson;
		
		Vector3 mRotation{ 0.0f };
		Vector3 mPosition{ 0.0f };
		Vector::Vector2 mMousePos{ 0.0f };

		float mRotationSpeed = 1.0f;
		float mTranslationSpeed = 2.0f;
		float mZoomSpeed = 1.0f;
		float mZoom = 0.0f;
		bool mUpdated = false;
		
		struct
		{
			bool Left  = false;
			bool Right = false;
			bool Up    = false;
			bool Down  = false;
		} mKeys;

		struct
		{
			bool Left   = false;
			bool Right  = false;
			bool Middle = false;
		} mMouseButtons;

		struct
		{
			Vector::Vector2 AxisLeft  = Vector::Vector2(0.0f);
			Vector::Vector2 AxisRight = Vector::Vector2(0.0f);
		} mGamepadState;

		struct TouchPos
		{
			UINT32 x = 0;
			UINT32 y = 0;
		} mTouchPos;

	private:
		bool Moving() const;
		void UpdateViewMatrix();
	};
}

