#pragma once

#include "Core.h"
#include "Math/Vector.h"

namespace Immortal
{

struct Ray
{
public:
    Ray() {}

    Ray(const Vector3 &origin, const Vector3 &direction) :
        origin{ origin },
        direction{ direction }
    {}

    Vector3 At(float t) const
    {
        return origin + t * direction;
    }

public:
    Vector3 origin;
    Vector3 direction;
};

}
