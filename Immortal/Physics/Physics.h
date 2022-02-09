#pragma once

#include "Core.h"
#include "Math/Vector.h"

namespace Immortal
{

namespace Physics
{

	struct RaycastHit
	{
		float Distance;
		Vector3 Point;
		Vector3 Normal;
	};
}

}

