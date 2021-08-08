#include "impch.h"
#include "Vector.h"

namespace Immortal {

	namespace Vector
	{
		bool DecomposeTransform(const mat4& transform, Vector3& position, Vector3& rotation, Vector3& scale)
		{
			using T = float;
			mat4 LocalMatrix(transform);

			// Normalize the matrix.
			if (EpsilonEqual(LocalMatrix[3][3], static_cast<float>(0), Epsilon<T>()))
				return false;

			// First, isolate perspective.  This is the messiest.
			if (
				EpsilonNotEqual(LocalMatrix[0][3], static_cast<T>(0), Epsilon<T>()) ||
				EpsilonNotEqual(LocalMatrix[1][3], static_cast<T>(0), Epsilon<T>()) ||
				EpsilonNotEqual(LocalMatrix[2][3], static_cast<T>(0), Epsilon<T>()))
			{
				// Clear the perspective partition
				LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<T>(0);
				LocalMatrix[3][3] = static_cast<T>(1);
			}

			// Next take care of translation (easy).
			position = Vector3(LocalMatrix[3]);
			LocalMatrix[3] = Vector4(0, 0, 0, LocalMatrix[3].w);

			Vector3 Row[3] = { Vector3(0.0f), Vector3(0.0f), Vector3(0.0f) };

			// Now get scale and shear.
			for (int i = 0; i < 3; ++i)
			{
				for (int j = 0; j < 3; ++j)
				{
					Row[i][j] = LocalMatrix[i][j];
				}
			}

			// Compute X scale factor and normalize first row.
			scale.x = Length(Row[0]);
			Row[0] = Detail::scale(Row[0], static_cast<T>(1));
			scale.y = Length(Row[1]);
			Row[1] = Detail::scale(Row[1], static_cast<T>(1));
			scale.z = Length(Row[2]);
			Row[2] = Detail::scale(Row[2], static_cast<T>(1));

			// At this point, the matrix (in rows[]) is orthonormal.
			// Check for a coordinate system flip.  If the determinant
			// is -1, then negate the matrix and the scaling factors.
#if 0
			Vector3 Pdum3;

			Pdum3 = cross(Row[1], Row[2]); // v3Cross(row[1], row[2], Pdum3);
			if (dot(Row[0], Pdum3) < 0)
			{
				for (length_t i = 0; i < 3; i++)
				{
					scale[i] *= static_cast<T>(-1);
					Row[i] *= static_cast<T>(-1);
				}
			}
#endif

			rotation.y = asin(-Row[0][2]);
			if (cos(rotation.y) != 0) {
				rotation.x = atan2(Row[1][2], Row[2][2]);
				rotation.z = atan2(Row[0][1], Row[0][0]);
			}
			else {
				rotation.x = atan2(-Row[2][0], Row[1][1]);
				rotation.z = 0;
			}

			return true;
		}
	}

}