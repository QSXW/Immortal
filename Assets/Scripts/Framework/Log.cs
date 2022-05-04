using System.Runtime.CompilerServices;

namespace Immortal
{
    class Log
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static public void Info(string message);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static public void Error(string message);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static public void Debug(string message);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static public void Fatal(string message);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static public void Warn(string message);
    }
}
