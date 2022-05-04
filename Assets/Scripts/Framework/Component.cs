using System.Runtime.CompilerServices;

namespace Immortal
{
    public abstract class Component
    {
        public GameObject GameObject { get; set; }
    }

    class TagComponent : Component
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern string __Get(ulong scene, int id);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void __Set(ulong scene, int id, string tag);

        public string Tag
        {
            get => __Get(GameObject.Scene, GameObject.Id);
            set => __Set(GameObject.Scene, GameObject.Id, value);
        }
    }

    class TransformComponent : Component
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void __GetPosition(ulong scene, int id, out Vector3 v);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void __SetPosition(ulong scene, int id, ref Vector3 v);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void __GetRotation(ulong scene, int id, out Vector3 v);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void __SetRotation(ulong scene, int id, ref Vector3 v);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void __GetScale(ulong scene, int id, out Vector3 v);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void __SetScale(ulong scene, int id, ref Vector3 v);

        public Vector3 Position
        {
            get
            {
                __GetPosition(GameObject.Scene, GameObject.Id, out Vector3 pos);
                return pos;
            }
            set
            {
                __SetPosition(GameObject.Scene, GameObject.Id, ref value);
            }
        }

        public Vector3 Rotation
        {
            get
            {
                __GetRotation(GameObject.Scene, GameObject.Id, out Vector3 pos);
                return pos;
            }
            set
            {
                __SetRotation(GameObject.Scene, GameObject.Id, ref value);
            }
        }

        public Vector3 Scale
        {
            get
            {
                __GetScale(GameObject.Scene, GameObject.Id, out Vector3 pos);
                return pos;
            }
            set
            {
                __SetScale(GameObject.Scene, GameObject.Id, ref value);
            }
        }
    }
}
