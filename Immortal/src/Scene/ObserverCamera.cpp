#include "impch.h"
#include "ObserverCamera.h"

#include "Framework/Math.h"
#include "Framework/Input.h"

namespace Immortal
{
	void ObserverCamera::UpdateViewMatrix() NOEXCEPT
	{
		Vector::Matrix4 rotationMatrix    = Vector::Rotate(mRotation);
		Vector::Matrix4 translationMatrix = Vector::Translate(mPosition);

		if (mType == Type::FirstPerson)
		{
			Camera::mView = rotationMatrix * translationMatrix;
		}
		else
		{
			Camera::mView = translationMatrix * rotationMatrix;
		}

		mUpdated = true;
	}

	void ObserverCamera::OnUpdate(float deltaTime) NOEXCEPT
	{
		if (Input::IsKeyPressed(KeyCode::A))
		{
			mKeys.Left = true;
		}
		if (Input::IsKeyPressed(KeyCode::D))
		{
			mKeys.Right = true;
		}
		if (Input::IsKeyPressed(KeyCode::W))
		{
			mKeys.Up = true;
		}
		if (Input::IsKeyPressed(KeyCode::S))
		{
			mKeys.Down = true;
		}

		if (Input::IsMouseButtonPressed(MouseCode::Left))
		{
			mMouseButtons.Left = true;
		}
		if (Input::IsMouseButtonPressed(MouseCode::Right))
		{
			mMouseButtons.Right = true;
		}
		if (Input::IsMouseButtonPressed(MouseCode::Middle))
		{
			mMouseButtons.Middle = true;
		}

		Vector::Vector2 mousePos = Input::GetMousePosition();
		INT32 dx = (INT32)(mMousePos.x - mousePos.x);
		INT32 dy = (INT32)(mMousePos.y - mousePos.y);

		if (mMouseButtons.Left)
		{
			mRotation += Vector::Vector3(dy * mRotationSpeed * deltaTime, -dx * mRotationSpeed * deltaTime, 0.0f);
		}
		if (mMouseButtons.Right)
		{
			mZoom += dy * .005f * mZoomSpeed * deltaTime;
			mPosition += Vector::Vector3(-0.0f, 0.0f, mZoom);
		}
		if (mMouseButtons.Middle)
		{
			mPosition += Vector::Vector3(-dx * 0.01f, -dy * 0.01f, 0.0f);
		}

		mMouseButtons.Left   = false;
		mMouseButtons.Right  = false;
		mMouseButtons.Middle = false;
		mMousePos.x = mousePos.x;
		mMousePos.y = mousePos.y;

		this->Update(deltaTime);

		mKeys.Left  = false;
		mKeys.Right = false;
		mKeys.Up    = false;
		mKeys.Down  = false;
	}

	void ObserverCamera::Update(float deltaTime) NOEXCEPT
	{
		mUpdated = false;
		if (mType == Type::FirstPerson)
		{
			if (Moving())
			{
				Vector::Vector3 front = Vector::Normalize(Vector::Vector3 {
					-Math::Cos(Vector::Radians(mRotation.x)) * Math::Sin(Vector::Radians(mRotation.y)),
					 Math::Sin(Vector::Radians(mRotation.x)),
					 Math::Cos(Vector::Radians(mRotation.x)) * Math::Cos(Vector::Radians(mRotation.y))
				});

				float moveSpeed = deltaTime * mTranslationSpeed;

				if (mKeys.Up)
				{
					mPosition += front * moveSpeed;
				}
				if (mKeys.Down)
				{
					mPosition -= front * moveSpeed;
				}
				if (mKeys.Left)
				{
					mPosition -= Vector::Normalize(Vector::Cross(front, Vector::Vector3(0.0f, 1.0f, 0.0f))) * moveSpeed;
				}
				if (mKeys.Right)
				{
					mPosition += Vector::Normalize(Vector::Cross(front, Vector::Vector3(0.0f, 1.0f, 0.0f))) * moveSpeed;
				}
			}
		}
		
		UpdateViewMatrix();
	}

	bool ObserverCamera::UpdateGamepad(Vector::Vector2 axisLeft, Vector::Vector2 axisRight, float deltaTime)
	{
		bool changed = false;

		if (mType == Type::FirstPerson)
		{
			// Use the common console thumbstick layout
			// Left = view, right = move
			const float deadZone = 0.0015f;
			const float range = 1.0f - deadZone;

			Vector::Vector3 front = Vector::Normalize(Vector::Vector3{
					-Math::Cos(Vector::Radians(mRotation.x)) * Math::Sin(Vector::Radians(mRotation.y)),
					 Math::Sin(Vector::Radians(mRotation.x)),
					 Math::Cos(Vector::Radians(mRotation.x)) * Math::Cos(Vector::Radians(mRotation.y))
				});

			float moveSpeed = deltaTime * mTranslationSpeed * 2.0f;
			float newRotationSpeed = deltaTime * mRotationSpeed * 50.0f;

			// Move
			if (fabsf(axisLeft.y) > deadZone)
			{
				float pos = (fabsf(axisLeft.y) - deadZone) / range;
				mPosition -= front * pos * ((axisLeft.y < 0.0f) ? -1.0f : 1.0f) * moveSpeed;
				changed = true;
			}
			if (fabsf(axisLeft.x) > deadZone)
			{
				float pos = (fabsf(axisLeft.x) - deadZone) / range;
				mPosition += Vector::Normalize(Vector::Cross(front, Vector::Vector3(0.0f, 1.0f, 0.0f))) * pos * ((axisLeft.x < 0.0f) ? -1.0f : 1.0f) * moveSpeed;
				changed = true;
			}

			// Rotate
			if (fabsf(axisRight.x) > deadZone)
			{
				float pos = (fabsf(axisRight.x) - deadZone) / range;
				mRotation.y += pos * ((axisRight.x < 0.0f) ? -1.0f : 1.0f) * newRotationSpeed;
				changed = true;
			}
			if (fabsf(axisRight.y) > deadZone)
			{
				float pos = (fabsf(axisRight.y) - deadZone) / range;
				mRotation.x -= pos * ((axisRight.y < 0.0f) ? -1.0f : 1.0f) * newRotationSpeed;
				changed = true;
			}
		}
		else
		{
			// todo: move code from example base class for look-at
		}

		if (changed)
		{
			UpdateViewMatrix();
		}

		return changed;
	}

	inline bool ObserverCamera::Moving() const NOEXCEPT
	{
		return mKeys.Left || mKeys.Right || mKeys.Up || mKeys.Down;
	}
}
