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
		bool UpdateGamepad(Vector2 axisLeft, Vector2 axisRight, float deltaTime);

	private:
		ObserverCamera::Type type = ObserverCamera::Type::FirstPerson;
		
		Vector3 rotation{ 0.0f };
		Vector3 position{ 0.0f };
		Vector2 mousePos{ 0.0f };

		float rotationSpeed = 1.0f;
		float translationSpeed = 8.0f;
		float zoomSpeed = 1.0f;
		float zoom = 0.0f;
		bool updated = false;
		
		struct
		{
			bool Left  = false;
			bool Right = false;
			bool Up    = false;
			bool Down  = false;
		} keys;

		struct
		{
			bool Left   = false;
			bool Right  = false;
			bool Middle = false;
		} mouseButtons;

		struct
		{
			Vector2 AxisLeft  = Vector2(0.0f);
			Vector2 AxisRight = Vector2(0.0f);
		} gamepadState;

		struct TouchPos
		{
			UINT32 x = 0;
			UINT32 y = 0;
		} touchPos;

	private:
		bool Moving() const;
		void UpdateViewMatrix();
	};
}

