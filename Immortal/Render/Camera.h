#pragma once

#include "Core.h"
#include "Math/Vector.h"
#include "Event/MouseEvent.h"

namespace Immortal
{

class Camera
{
public:
    enum class ProjectionType
    { 
        Perspective  = 0,
        Orthographic = 1
    };

public:
    Camera(ProjectionType type = ProjectionType::Perspective) :
        projectionType{ type },
        clipNear{ 0.1f },
        clipFar{ 200.0f }
    {

    }

    Camera(const Matrix4 &prj) :
        projection{ prj },
        clipNear{ 0.1f },
        clipFar{ 200.0f }
    {

    }

    virtual ~Camera() = default;

    const Matrix4 Projection() const
    { 
        return projection;
    }

    const Matrix4 &View() const
    { 
        return view;
    }

    void SetProjection(const Matrix4 &prj)
    {
        projection = prj;
    }

    float Exposure() const
    {
        return exposure;
    }

    float &Exposure()
    {
        return exposure;
    }

    virtual Matrix4 ViewProjection() const
    { 
        return projection * view;
    }

	/** World-space camera position (inverse(view) * origin). Override if view is not a standard world-to-camera matrix. */
	virtual Vector3 GetWorldPosition() const
	{
		return Vector3{glm::vec3{Vector::Inverse(view)[3]}};
	}

    virtual void SetViewportSize(Vector2 viewportSize)
    {

    }

    virtual void OnUpdate(const float &deltaTime = Time::DeltaTime)
    {

    }

    virtual void OnEvent(Event &e)
    {
    
    }

    virtual bool OnMouseScrolled(MouseScrolledEvent &e)
    {
        return true;
    }

    bool IsOrthographic() const
    {
        return projectionType == ProjectionType::Orthographic;
    }

	/** Shadow / CSM frustum fitting reads these (set whenever projection clip planes change). */
	float ClipNear() const
	{
		return clipNear;
	}

	float ClipFar() const
	{
		return clipFar;
	}

	void SetClipPlanes(float nearPlane, float farPlane)
	{
		clipNear = nearPlane > 1e-6f ? nearPlane : 1e-6f;
		clipFar = farPlane > clipNear + 1e-3f ? farPlane : clipNear + 1e-3f;
	}

protected:
    ProjectionType projectionType = ProjectionType::Perspective;

    Matrix4 view{ 1.0f };

    Matrix4 projection{ 1.0f };

    float exposure{ 0.8f };

	float clipNear = 0.1f;
	float clipFar = 200.0f;
};

}
