#include "impch.h"
#include "EditorCamera.h"

#include "Framework/Input.h"

namespace Immortal {

	EditorCamera::EditorCamera(const Vector::Matrix4 & projection)
		: Camera(projection)/*, mRotation{ 90.0f, 0.0f, 0.0f }, mFocalPoint(0.0f)*/
	{
		mDistance = Vector::Distance(Vector::Vector3{ -5.0f, 5.0f, 5.0f }, mFocalPoint);

		mYaw = 0.0f;
		mPitch = 0.0f;
		/*mYaw = 3.0f * Vector::PI / 4.0f;
		mPitch = Vector::PI / 4.0f;*/

		UpdateView();
	}

	void EditorCamera::Focus(const Vector::Vector3 & focusPoint)
	{
		mFocalPoint = focusPoint;
		if (mDistance > mMinFocusDistance)
		{
			float distance = mDistance - mMinFocusDistance;
			MouseZoom(distance / ZoomSpeed());
			UpdateView();
		}
	}

	void EditorCamera::OnUpdate()
	{
		if (Input::IsKeyPressed(KeyCode::LeftAlt))
		{
			const Vector::Vector2 &mouse = Input::GetMousePosition();
			Vector::Vector2 delta = (mouse - mInitialPosition) * 0.003f;
			mInitialPosition = mouse;

			if (Input::IsMouseButtonPressed(MouseCode::Middle))
			{
				MousePan(delta);
			}
			else if (Input::IsMouseButtonPressed(MouseCode::Left))
			{
				MouseRotate(delta);
			}
			else if (Input::IsMouseButtonPressed(MouseCode::Right))
			{
				MouseZoom(delta.y);
			}
		}

		UpdateView();
	}

	void EditorCamera::OnEvent(Event & e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(std::bind(&EditorCamera::OnMouseScroll, this, std::placeholders::_1));
	}

	Vector::Vector3 EditorCamera::UpDirection()
	{
		return Vector::Rotate(Orientation(), Vector::Vector3(0.0f, 1.0f, 0.0f));
	}

	Vector::Vector3 EditorCamera::RightDirection()
	{
		return Vector::Rotate(Orientation(), Vector::Vector3(1.0f, 0.0f, 0.0f));
	}

	Vector::Vector3 EditorCamera::ForwardDirection()
	{
		return Vector::Rotate(Orientation(), Vector::Vector3(0.0f, 0.0f, -1.0f));
	}

	Vector::Quaternion EditorCamera::Orientation() const
	{
		return Vector::Quaternion(Vector::Vector3(-mPitch, -mYaw, 0.0f));
	}

	void EditorCamera::UpdateView()
	{
		mPosition = CalculatePosition();
		Vector::Quaternion orientation = Orientation();
		mRotation = Vector::EulerAngles(orientation) * (180.0f / Vector::PI);
		mView = Vector::Translate(mPosition) * Vector::ToMatrix4(orientation);
		mView = Vector::Inverse(mView);
	}

	bool EditorCamera::OnMouseScroll(MouseScrolledEvent & e)
	{
		float delta = e.GetYOffset() * 0.1f;
		MouseZoom(delta);
		UpdateView();
		return false;
	}

	void EditorCamera::MousePan(const Vector::Vector2 & delta)
	{
		Vector::Vector2 speed = PanSpeed();
		mFocalPoint += -RightDirection() * delta.x * speed.x * mDistance;
		mFocalPoint += UpDirection() * delta.y * speed.y * mDistance;
	}

	void EditorCamera::MouseRotate(const Vector::Vector2 & delta)
	{
		float yawSign = UpDirection().y < 0 ? -1.0f : 1.0f;
		mYaw += yawSign * delta.x * RotateSpeed();
		mPitch += delta.y * RotateSpeed();
	}

	void EditorCamera::MouseZoom(float delta)
	{
		mDistance -= delta * ZoomSpeed();
		if (mDistance < 1.0f)
		{
			mFocalPoint += ForwardDirection();
			mDistance = 1.0f;
		}
	}

	Vector::Vector3 EditorCamera::CalculatePosition()
	{
		return mFocalPoint - ForwardDirection() * mDistance;
	}

	template <class T>
	inline float SolveSpeedFactorFromQuadraticFormula(T &x)
	{
		return 0.0366f * (x * x) - 0.1778f * x + 0.3021f;
	}

	/* Max speed = 2.4f, this function build on magic. */
	Vector::Vector2 EditorCamera::PanSpeed() const
	{
		float x = std::min(mViewportSize.x / 1000.0f, 2.4f);
		float xFactor = SolveSpeedFactorFromQuadraticFormula(x);

		float y = std::min(mViewportSize.y / 1000.0f, 2.4f);
		float yFactor = SolveSpeedFactorFromQuadraticFormula(y);

		return { xFactor, yFactor };
	}

	constexpr float EditorCamera::RotateSpeed() const
	{
		return 0.8f;
	}

	/* max speed = 100.0f */
	float EditorCamera::ZoomSpeed() const
	{
		float distance = mDistance * 0.2f;
		distance = std::max(distance, 0.0f);
		float speed = distance * distance;
		return std::min(speed, 100.0f);
	}

	void EditorCamera::SetViewportSize(const Vector::Vector2 &size)
	{
		mViewportSize = size;
		SetProjection(Vector::PerspectiveFOV(FOV, mViewportSize.x, mViewportSize.y, (float)0.1, 1000));
	}
}