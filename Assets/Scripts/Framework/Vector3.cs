

using System.Runtime.InteropServices;

namespace Immortal
{
	[StructLayout(LayoutKind.Sequential)]
	public struct Vector3
	{
		public float x;
		public float y;
		public float z;

		public static readonly Vector3 Up      = new(0, 1, 0);
		public static readonly Vector3 Forward = new(0, 0, -1);
		public static readonly Vector3 Right   = new(1, 0, 0);

		public Vector3(float x, float y, float z)
		{
			this.x = x; this.y = y; this.z = z;
		}

		public Vector3(float scalar)
		{
			x = y = z = scalar;
		}

		public Vector3(Vector4 v)
		{
			x = v.x;
			y = v.y;
			z = v.z;
		}

		public static Vector3 operator*(Vector3 v, float scalar)
		{
			return new Vector3(v.x * scalar, v.y * scalar, v.z * scalar);
		}

		public static Vector3 operator +(Vector3 v, float scalar)
		{
			return new Vector3(v.x + scalar, v.y + scalar, v.z + scalar);
		}

		public static Vector3 operator -(Vector3 v, float scalar)
		{
			return new Vector3(v.x - scalar, v.y - scalar, v.z - scalar);
		}
	}
}
