#include "impch.h"
#include "OrthographicCamera.h"

namespace Immortal {

	OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top)
		: mProjectionMatrix(Vector::Ortho(left, right, bottom, top, -1.0f, 1.0f)),
		  mViewMatrix(1.0f)
	{
		mViewProjectionMatrix = mProjectionMatrix * mViewMatrix;
	}

	void OrthographicCamera::SetProjection(float left, float right, float bottom, float top)
	{
		mProjectionMatrix = Vector::Ortho(left, right, bottom, top, -1.0f, 1.0f);
		mViewProjectionMatrix = mProjectionMatrix * mViewMatrix;
	}

	void OrthographicCamera::SetProjection(Vector::mat4 projectionMatrix)
	{
		mProjectionMatrix = projectionMatrix;
		mViewProjectionMatrix = mProjectionMatrix * mViewMatrix;
	}

	void OrthographicCamera::ReCalculateViewMatrix()
	{
		// transformation matrix = tranlate( identity matrix(uint matrix)
		Vector::mat4 transform = Vector::Translate(mPosition) *
			Vector::Rotate(mRotation, Vector::Vector3(0, 0, 1));

		mViewMatrix = Vector::Inverse(transform);
		mViewProjectionMatrix = mProjectionMatrix * mViewMatrix;
	}

} 