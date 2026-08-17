#include "EditorCamera.h"
#include "Framework/Timer.h"
#include "Input.h"
#include "Math/Math.h"

#include <algorithm>

namespace Immortal
{

namespace
{
/* Orientation() feeds yaw/pitch into glm as radians. Orbit must stay below gimbal lock so ForwardDirection stays stable. */
constexpr float kMaxPitchRad = 1.553343f; /* ~89 deg */
/* Fast pointer motion or long frames otherwise apply huge yaw/pitch in one tick — breaks CSM inv-frustum numerics (full-screen shadow). */
constexpr float kMaxOrbitStepRad = 0.52f; /* ~30 deg / frame */
constexpr float kMaxPointerDelta = 1.25f; /* after *0.003f; caps ~416 px equivalent */
constexpr float kHermiteSmoothRate = 18.0f;
}

EditorCamera::EditorCamera(float fov, float width, float height, float zNear, float zFar) :
    EditorCamera{Vector::PerspectiveFOV(Vector::Radians(fov), width, height, zNear, zFar)}
{
	FOV = fov;
	SetClipPlanes(zNear, zFar);
}

EditorCamera::EditorCamera(const Matrix4 &projection) :
    Camera{ projection }
{
    distance = Vector3{ -0.8f, 0.8f, 0.8f }.Distance(focalPoint);

    yaw = 0.0f;
    pitch = 0.0f;

    UpdateView();

	SetClipPlanes(0.1f, 1000.0f);
}

void EditorCamera::Focus(const Vector3 & focusPoint)
{
    targetPanDelta = {};
    targetRotateDelta = {};
    targetZoomDelta = 0.0f;
    hasLastMouseForOrbit = false;

    focalPoint = focusPoint;
    if (distance > minFocusDistance)
	{
		targetZoomDelta = (distance - minFocusDistance) / ZoomSpeed();
        //MouseZoom((distance - minFocusDistance) / ZoomSpeed());
        //UpdateView();
    }
}

void EditorCamera::OnUpdate(const float &deltaTime)
{
    if (!Input::IsKeyPressed(KeyCode::LeftAlt))
    {
        hasLastMouseForOrbit = false;
    }

	const float k = deltaTime * kHermiteSmoothRate;

    auto pullAxis = [k](float &target) -> float
    {
        if (std::abs(target) < 1.0e-6f)
        {
            target = 0.0f;
            return 0.0f;
        }
        const float step = Math::HermiteLerp(0.0f, target, k);
        target -= step;
        if (std::abs(target) < 1.0e-6f)
        {
            target = 0.0f;
        }
        return step;
    };

    if (std::abs(targetPanDelta.x) + std::abs(targetPanDelta.y) > 1.0e-6f)
    {
        const float px = pullAxis(targetPanDelta.x);
        const float py = pullAxis(targetPanDelta.y);
        MousePan({ px, py });
    }

    if (std::abs(targetRotateDelta.x) + std::abs(targetRotateDelta.y) > 1.0e-6f)
    {
        const float rx = pullAxis(targetRotateDelta.x);
        const float ry = pullAxis(targetRotateDelta.y);
        MouseRotate({ rx, ry });
    }

    if (std::abs(targetZoomDelta) > 1.0e-6f)
    {
        MouseZoom(pullAxis(targetZoomDelta));
    }

    UpdateView();
}

void EditorCamera::OnEvent(Event & e)
{
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<MouseMoveEvent>(std::bind(&EditorCamera::OnMouseMoved, this, std::placeholders::_1));
    dispatcher.Dispatch<MouseButtonPressedEvent>(std::bind(&EditorCamera::OnMouseButtonPressed, this, std::placeholders::_1));
    dispatcher.Dispatch<MouseScrolledEvent>(std::bind(&EditorCamera::OnMouseScroll, this, std::placeholders::_1));
}

bool EditorCamera::OnMouseMoved(MouseMoveEvent &e)
{
    if (!Input::IsKeyPressed(KeyCode::LeftAlt))
    {
        hasLastMouseForOrbit = false;
        return false;
    }

    /* Event position: OnUpdate runs before ProcessEvents; using Input here would match lastMouse set in OnUpdate and yield zero delta. */
    Vector2 mouse{ e.GetX(), e.GetY() };
    if (!hasLastMouseForOrbit)
    {
        lastMouseForOrbit = mouse;
        hasLastMouseForOrbit = true;
        return false;
    }

    Vector2 delta = (mouse - lastMouseForOrbit) * 0.002f;
    lastMouseForOrbit = mouse;
    delta.x = std::clamp(delta.x, -kMaxPointerDelta, kMaxPointerDelta);
    delta.y = std::clamp(delta.y, -kMaxPointerDelta, kMaxPointerDelta);

    if (Input::IsMouseButtonPressed(MouseCode::Middle))
    {
        targetPanDelta += delta;
    }
    else if (Input::IsMouseButtonPressed(MouseCode::Left))
    {
        targetRotateDelta += delta;
    }
    else if (Input::IsMouseButtonPressed(MouseCode::Right))
    {
        targetZoomDelta += delta.y;
    }

    return false;
}

bool EditorCamera::OnMouseButtonPressed(MouseButtonPressedEvent &e)
{
    const MouseCode b = e.GetMouseButton();
    if (b != MouseCode::Middle && b != MouseCode::Left && b != MouseCode::Right)
    {
        return false;
    }

    /* Do not seed lastMouse from Input: MouseMove uses event coords; mixing spaces breaks the first deltas. */
    hasLastMouseForOrbit = false;

    return false;
}

Vector3 EditorCamera::UpDirection()
{
    return Vector::Rotate(Orientation(), Vector3(0.0f, 1.0f, 0.0f));
}

Vector3 EditorCamera::RightDirection()
{
    return Vector::Rotate(Orientation(), Vector3(1.0f, 0.0f, 0.0f));
}

Vector3 EditorCamera::ForwardDirection()
{
    return Vector::Rotate(Orientation(), Vector3(0.0f, 0.0f, -1.0f));
}

Quaternion EditorCamera::Orientation() const
{
    return Quaternion{ Vector3{ -pitch, -yaw, 0.0f } };
}

void EditorCamera::UpdateView()
{
    pitch = std::clamp(pitch, -kMaxPitchRad, kMaxPitchRad);
    position = CalculatePosition();
    Quaternion orientation = Orientation();
    rotation = Vector::EulerAngles(orientation) * (float)(180.0f / Math::PI);
    view     = Vector::Translate(position) * Vector::ToMatrix4(orientation);
    view     = Vector::Inverse(view);
}

bool EditorCamera::OnMouseScroll(MouseScrolledEvent & e)
{
    targetZoomDelta += e.GetOffsetY() * Time::DeltaTime;
    return false;
}

void EditorCamera::MousePan(const Vector::Vector2 & delta)
{
    Vector::Vector2 speed = PanSpeed();
    focalPoint += -RightDirection() * delta.x * speed.x * distance;
    focalPoint += UpDirection() * delta.y * speed.y * distance;
}

void EditorCamera::MouseRotate(const Vector::Vector2 & delta)
{
    float speed = 115.0f * Time::DeltaTime;
    float yawSign = UpDirection().y < 0 ? -1.0f : 1.0f;
    float dYaw = yawSign * delta.x * speed;
    float dPitch = delta.y * speed;
    dYaw = std::clamp(dYaw, -kMaxOrbitStepRad, kMaxOrbitStepRad);
    dPitch = std::clamp(dPitch, -kMaxOrbitStepRad, kMaxOrbitStepRad);
    yaw += dYaw;
    pitch += dPitch;
    pitch = std::clamp(pitch, -kMaxPitchRad, kMaxPitchRad);
}

void EditorCamera::MouseZoom(float delta)
{
    distance -= delta * ZoomSpeed();
    if (distance < 1.0f)
    {
        focalPoint += ForwardDirection();
        distance = 1.0f;
    }
}

Vector3 EditorCamera::CalculatePosition()
{
    return focalPoint - ForwardDirection() * distance;
}

template <class T>
inline float SpeedFactor(T &x)
{
    return 0.0366f * (x * x) - 0.1778f * x + 0.3021f;
}

Vector2 EditorCamera::PanSpeed() const
{
    float deltaTime = Time::DeltaTime;
    float x = std::min(viewportSize.x, 240.0f) * deltaTime;
    float xFactor = SpeedFactor(x);
    xFactor = Math::Lerp(0.0f, 1.5f, xFactor);

    float y = std::min(viewportSize.y, 240.0f) *deltaTime;
    float yFactor = SpeedFactor(y);
    yFactor = Math::Lerp(0.0f, 1.5f, yFactor);

    return { xFactor, yFactor };
}

float EditorCamera::ZoomSpeed() const
{
    float speed = distance * 50.0f * Time::DeltaTime;
    speed = std::max(speed, 0.0f);
    return std::min(speed * speed, 100.0f);
}

void EditorCamera::SetViewportSize(Vector2 size)
{
    viewportSize = size;
	SetProjection(Vector::PerspectiveFOV(Vector::Radians(FOV), viewportSize.x, viewportSize.y, clipNear, clipFar));
}

void EditorCamera::ExportOrbitSnapshot(Vector3 &outFocal, float &outDist, float &outPitchDeg, float &outYawDeg, float &outFovDeg) const
{
	outFocal     = focalPoint;
	outDist      = distance;
	outPitchDeg  = pitch;
	outYawDeg    = yaw;
	outFovDeg    = FOV;
}

void EditorCamera::ImportOrbitSnapshot(const Vector3 &focal, float dist, float pitchDeg, float yawDeg, float fovDeg, float zNear, float zFar)
{
	targetPanDelta = {};
	targetRotateDelta = {};
	targetZoomDelta = 0.0f;
	hasLastMouseForOrbit = false;

	focalPoint = focal;
	distance   = dist;
	pitch      = pitchDeg;
	yaw        = yawDeg;
	FOV        = fovDeg;
	UpdateView();
	if (viewportSize.x > 0.0f && viewportSize.y > 0.0f)
	{
		SetProjection(Vector::PerspectiveFOV(Vector::Radians(FOV), viewportSize.x, viewportSize.y, zNear, zFar));
		SetClipPlanes(zNear, zFar);
	}
}
}
