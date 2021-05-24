#pragma once

#include "ImmortalCore.h"
#include "Immortal/Core/Vector.h"

#include <cmath>

namespace Immortal {

	class IMMORTAL_API ColorSpace
	{
	public:
		static Vector::Vector3 HSVToRGB(const Vector::Vector3 &HSV)
		{
            float C = HSV.z * HSV.y;
            float X = (float)(C * (1 - (float)abs(fmod(HSV.x, 2) - 1)));
            float m = HSV.z- C;

            float r, g, b;
            if (HSV.x >= 0 && HSV.x < 1.0f)
            {
                r = C, g = X, b = 0;
            }
            else if (HSV.x < 2.0f)
            {
                r = X, g = C, b = 0;
            }
            else if (HSV.x < 3.0f)
            {
                r = 0, g = C, b = X;
            }
            else if (HSV.x < 4.0f)
            {
                r = 0, g = X, b = C;
            }
            else if (HSV.x < 5.0f)
            {
                r = X, g = 0, b = C;
            }
            else
            {
                r = C, g = 0, b = X;
            }

            return Vector::Vector3{ r + m, g + m, b + m };
		}
	};


}
