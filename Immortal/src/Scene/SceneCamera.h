#pragma once

#include "ImmortalCore.h"
#include "Render/Camera.h"

namespace Immortal {

	class SceneCamera : public Camera
	{
	public:
		SceneCamera();
		virtual ~SceneCamera();

		void SetPerspective(float verticalFOV, float nearClip = 0.1f, float farClip = 10000.0f);
		void SetOrthographic(float size, float nearClip = -1.0f, float farClip = 1.0f);

		void SetViewportSize(float width, float height);
		void SetViewportSize(const Vector::Vector2 &size);

		void SetPerspectiveVerticalFOV(float verticalFov) { mPerspectiveFOV = Vector::Radians(verticalFov); }
		float PerspectiveVerticalFOV() const { return Vector::Degrees(mPerspectiveFOV); }
		void SetPerspectiveNearClip(float nearClip) { mPerspectiveNear = nearClip; }
		float &PerspectiveNearClip() { return mPerspectiveNear; }
		void SetPerspectiveFarClip(float farClip) { mPerspectiveFar = farClip; }
		float &PerspectiveFarClip() { return mPerspectiveFar; }

		void SetOrthographicSize(float size) { mOrthographicSize = size; }
		float OrthographicSize() const { return mOrthographicSize; }
		float &OrthographicSize() { return mOrthographicSize; }
		void SetOrthographicNearClip(float nearClip) { mOrthographicNear = nearClip; }
		float &OrthographicNearClip() { return mOrthographicNear; }
		void SetOrthographicFarClip(float farClip) { mOrthographicFar = farClip; }
		float &OrthographicFarClip() { return mOrthographicFar; }

		void SetProjectionType(ProjectionType type) { mType = type; }
		ProjectionType Type() const { return mType; }

		void SetTransform(const Vector::Matrix4 &transform)
		{
			mView = Vector::Inverse(transform);
		}

	protected:
		ProjectionType mType = ProjectionType::Perspective;

		float mPerspectiveFOV{ Vector::Radians(90.0f) };
		float mPerspectiveNear{ 0.1f };
		float mPerspectiveFar{ 1000.0f };

		float mOrthographicSize{ 10.0f };
		float mOrthographicNear{ -1.0f };
		float mOrthographicFar{ 1.0f };

	};

}

