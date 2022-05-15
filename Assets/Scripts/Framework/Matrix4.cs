using System.Runtime.InteropServices;

namespace Immortal
{
	[StructLayout(LayoutKind.Sequential)]
	public struct Matrix4
	{
		public Vector4 x;
		public Vector4 y;
		public Vector4 z;	
		public Vector4 w;

		Vector4 Position { get { return x; } set { x = value; } }

		Vector4 Rotation { get { return y; } set { y = value; } }

		Vector4 Scale { get { return z; } set { z = value; } }
	}
}
