using System.Runtime.CompilerServices;

namespace Immortal
{
    public class GameObject
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static bool __HasComponent(ulong scene, int objectId, Type type);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static void __AddComponent(ulong scene, int objectId, Type type);

        private ulong scene;
        private int id = 0;

        private void SetId_(int v, ulong s)
        {
            scene = s;
            id = v;
        }

        public int Id { get { return id; } }

        public ulong Scene
        { 
            get { return scene;  }
            set { scene = value; }
        }

        public GameObject()
        {
            SetId_(0, 0);
            Log.Debug("Calling default constructor");
        }

        public GameObject(int id)
        {
            this.id = id;
            Log.Debug("Create Object " + id);
        }

        virtual public void Update(float deltatime)
        {
            Log.Debug("object id is " + id.ToString());
            Log.Debug(deltatime.ToString());
        }

        virtual public void FixedUpdate(float deltaTime)
        {

        }

        virtual public void OnKeyDown(KeyCode code)
		{
            Log.Debug("You press key: " + code);
		}

        virtual public void OnButtonDown(MouseCode code)
        {

        }

        public bool HasComponent<T>() where T : Component, new()
        {
            return __HasComponent(scene, id, typeof(T));
        }

        public T AddComponent<T>() where T : Component, new()
        {
			T component = new()
			{
				GameObject = this
			};

			__AddComponent(scene, id, typeof(T));
            return component;
        }

        public T GetComponent<T>() where T : Component, new()
		{
            if (HasComponent<T>())
			{
                T component = new()
                {
                    GameObject = this
                };

                return component;
            }

            return null;
		}
            
    }
}
