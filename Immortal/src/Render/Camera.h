#pragma once

#include "ImmortalCore.h"

namespace Immortal
{
class Camera
{
public:
    enum class ProjectionType { Perspective = 0, Orthographic = 1 };

public:
    Camera() = default;
    Camera(const Vector::Matrix4 &projection)
        : mProjection(projection)
    {

    }

    virtual ~Camera() = default;

    const Vector::Matrix4 Projection() const
    { 
        return mProjection;
    }

    const Vector::Matrix4 &View() const
    { 
        return mView;
    }

    Vector::Matrix4 ViewProjection() const
    { 
        return mProjection * mView;
    }

    void SetProjection(const Vector::Matrix4 &projection)
    { 
        mProjection = projection;
    }

    float Exposure() const
    { 
        return exposure;
    }

    float &Exposure()
    {
        return exposure;
    }

protected:
    Vector::Matrix4 mView{ 1.0f };
    Vector::Matrix4 mProjection{ 1.0f };
    float exposure{ 0.8f };
};
}
