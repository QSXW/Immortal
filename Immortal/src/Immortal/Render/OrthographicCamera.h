#pragma once

#include "ImmortalCore.h"

namespace Immortal {

	class IMMORTAL_API OrthographicCamera
	{
	public:
		OrthographicCamera() = default;
		OrthographicCamera(float left, float right, float bottom, float top);

		void SetProjection(float left, float right, float bottom, float top);

		const Vector::Vector3 &Position() const { return mPosition; }
		
		void SetProjection(Vector::mat4 projectionMatrix);
		void setPosition(const Vector::Vector3 &position)
		{
			mPosition = position;
			ReCalculateViewMatrix();
		}
		
		void Set(const Vector::Vector3 &position, float rotation)
		{
			mPosition = position;
			mRotation = rotation;
			ReCalculateViewMatrix();
		}

		float Rotation() const { return mRotation; }
		void SetRotation(float rotation)
		{ 
			mRotation = rotation;
			ReCalculateViewMatrix();
		}

		const Vector::mat4 &ProjectionMatrix() const { return mProjectionMatrix; }
		const Vector::mat4 &ViewMatrix() const { return mViewMatrix; }
		const Vector::mat4 &ViewPorjectionMatrix() const { return mViewProjectionMatrix; }

	private:
		void ReCalculateViewMatrix();

	private:
		Vector::mat4 mProjectionMatrix;
		Vector::mat4 mViewMatrix;
		Vector::mat4 mViewProjectionMatrix;

		Vector::Vector3 mPosition = { 0.0f, 0.0f, 0.0f };
		float mRotation = 0.0f;
	};

}

