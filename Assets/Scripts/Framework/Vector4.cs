using System.Runtime.InteropServices;

namespace Immortal
{
	[StructLayout(LayoutKind.Sequential)]
	public struct Vector4
	{
		public float x;
		public float y;
		public float z;
		public float w;

		public static readonly Vector3 Up = new(0, 1, 0);
		public static readonly Vector3 Forward = new(0, 0, -1);
		public static readonly Vector3 Right = new(1, 0, 0);

		public Vector4(float x, float y, float z, float w)
		{
			this.x = x; this.y = y; this.z = z; this.w = w;
		}

		public Vector4(float scalar)
		{
			x = y = z = w = scalar;
		}

		public Vector4(Vector3 v)
		{
			x = v.x;
			y = v.y;
			z = v.z;
			w = 1.0f;
		}

		public static Vector4 operator *(Vector4 v, float scalar)
		{
			return new Vector4(v.x * scalar, v.y * scalar, v.z * scalar, v.w * scalar);
		}

		public static Vector4 operator +(Vector4 v, float scalar)
		{
			return new Vector4(v.x + scalar, v.y + scalar, v.z + scalar, v.w + scalar);
		}

		public static Vector4 operator -(Vector4 v, float scalar)
		{
			return new Vector4(v.x - scalar, v.y - scalar, v.z - scalar, v.w - scalar);
		}
	}
}
