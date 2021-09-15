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
    type = ProjectionType::Perspective;
    perspectiveFOV = Vector::Radians(verticalFOV);
    perspectiveFar = nearClip;
    perspectiveFar = farClip;
}

void SceneCamera::SetOrthographic(float size, float nearClip, float farClip)
{
    type = ProjectionType::Orthographic;
    orthographicSize = size;
    orthographicNear = nearClip;
    orthographicFar = farClip;
}

void SceneCamera::SetViewportSize(float width, float height)
{
    if (type == ProjectionType::Perspective)
    {
        projection = Vector::PerspectiveFOV(perspectiveFOV, (float)width, (float)height, perspectiveNear, perspectiveFar);
    }
    else if (type == ProjectionType::Orthographic)
    {
        float aspect = (float)width / (float)height;
        float w = orthographicSize * aspect;
        float h = orthographicSize;
        projection = Vector::Ortho(-w * 0.5f, w * 0.5f, -h * 0.5f, h * 0.5f, orthographicNear, orthographicFar);
    }
}

}
