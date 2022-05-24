#include "SceneCamera.h"

namespace Immortal
{

SceneCamera::SceneCamera()
{
    SetProjectionType(ProjectionType::Orthographic);
}

SceneCamera::~SceneCamera()
{

}

void SceneCamera::SetPerspective(float verticalFOV, float nearClip, float farClip)
{
    projectionType = ProjectionType::Perspective;
    perspectiveFOV = Vector::Radians(verticalFOV);
    perspectiveFar = nearClip;
    perspectiveFar = farClip;
}

void SceneCamera::SetOrthographic(float size, float nearClip, float farClip)
{
    projectionType = ProjectionType::Orthographic;
    orthographicSize = size;
    orthographicNear = nearClip;
    orthographicFar = farClip;
}

void SceneCamera::SetViewportSize(float width, float height)
{
    if (projectionType == ProjectionType::Perspective)
    {
        projection = Vector::PerspectiveFOV(perspectiveFOV, width, height, perspectiveNear, perspectiveFar);
    }
    else if (projectionType == ProjectionType::Orthographic)
    {
        float aspect = width / height;
        float w = orthographicSize * aspect;
        float h = orthographicSize;
        projection = Vector::Ortho(-w * 0.5f, w * 0.5f, -h * 0.5f, h * 0.5f, orthographicNear, orthographicFar);
    }
}

}
