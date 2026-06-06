/**
 * Copyright (C) 2022-2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "Component.h"

#include "Render/Camera.h"
#include "Math/Math.h"
#include "Math/Vector.h"

namespace Immortal
{

namespace
{

/** World unit: from surface toward bright side, using -R*Forward (same axis as TransformComponent::Forward convention). */
Vector3 AxisTowardLightFromLocalForward(const TransformComponent &transform)
{
	Vector4 d    = Vector::Mul(Vector::Rotate(transform.Rotation), TransformComponent::Forward, 0.0f);
	Vector3 axis = Vector::Normalize(Vector3{ d.x, d.y, d.z });
	return -axis;
}

}

Vector3 LightComponent::DefaultDirectionalLightDirection()
{
	return Vector::Normalize(Vector3{ 0.35f, 0.85f, 0.25f });
}

Vector3 LightComponent::DirectionWorld(const Camera &camera, const TransformComponent &transform) const
{
	switch (LightType)
	{
	case Type::Directional:
		/* No position in the light model: only rotation sets the sun direction. */
		return AxisTowardLightFromLocalForward(transform);
	case Type::Spot:
		/* Cone axis matches the same local Forward as directional; falloff/cone use Position + spotParams in deferred. */
		return AxisTowardLightFromLocalForward(transform);
	case Type::Point:
	default:
	{
		const Vector3 camPos = camera.GetWorldPosition();
		Vector3 w = transform.Position - camPos;
		const float len2 = w.x * w.x + w.y * w.y + w.z * w.z;
		if (len2 < 1e-18f)
		{
			return DefaultDirectionalLightDirection();
		}
		return Vector::Normalize(w);
	}
	}
}

}
