using System.Runtime.CompilerServices;

namespace Immortal
{
    class Scene
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static ulong Init();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static int CreateObject(ulong scene, string name);

        private readonly ulong _id;

        public Scene()
        {
            _id = Init();
            Log.Debug("Create Scene: " + _id);
        }

        public GameObject CreateGameObject(string name)
        {
            return new GameObject(CreateObject(_id, name));
        }
    }
}
