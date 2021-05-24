#include "impch.h"
#include "SceneCamera.h"

namespace Immortal
{
	SceneCamera::SceneCamera()
	{

	}

	SceneCamera::~SceneCamera()
	{

	}
	void SceneCamera::SetPerspective(float verticalFOV, float nearClip, float farClip)
	{
		mType = ProjectionType::Perspective;
		mPerspectiveFOV = Vector::Radians(verticalFOV);
		mPerspectiveFar = nearClip;
		mPerspectiveFar = farClip;
	}

	void SceneCamera::SetOrthographic(float size, float nearClip, float farClip)
	{
		mType = ProjectionType::Orthographic;
		mOrthographicSize = size;
		mOrthographicNear = nearClip;
		mOrthographicFar = farClip;
	}

	void SceneCamera::SetViewportSize(float width, float height)
	{
		if (mType == ProjectionType::Perspective)
		{
			mProjection = Vector::PerspectiveFOV(mPerspectiveFOV, (float)width, (float)height, mPerspectiveNear, mPerspectiveFar);
		}
		else if (mType == ProjectionType::Orthographic)
		{
			float aspect = (float)width / (float)height;
			float w = mOrthographicSize * aspect;
			float h = mOrthographicSize;
			mProjection = Vector::Ortho(-w * 0.5f, w * 0.5f, -h * 0.5f, h * 0.5f, mOrthographicNear, mOrthographicFar);
		}
	}

	void inline SceneCamera::SetViewportSize(const Vector::Vector2 &size)
	{
		this->SetViewportSize(size.x, size.y);
	}
}