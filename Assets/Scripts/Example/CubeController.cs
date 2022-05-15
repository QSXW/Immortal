using Immortal;

namespace Immortal
{
	internal class CubeController : GameObject
	{
		public float speed = 10.0f;

		override public void Update(float deltaTime)
		{
			TransformComponent transformComponent = GetComponent<TransformComponent>();
			var transform = transformComponent;
			var pos = transform.Position;

			if (Input.GetKeyDown(KeyCode.A))
			{
				pos.x -= deltaTime * speed;
			}
			if (Input.GetKeyDown(KeyCode.D))
			{
				pos.x += deltaTime * speed;
			}
			if (Input.GetKeyDown(KeyCode.W))
			{
				pos.y += deltaTime * speed;
			}
			if (Input.GetKeyDown(KeyCode.S))
			{
				pos.y -= deltaTime * speed;
			}

			var rot = transform.Rotation;
			if (Input.GetButtonDown(MouseCode.Button0))
			{
				rot.z += deltaTime * speed;
			}
			if (Input.GetButtonDown(MouseCode.Button1))
			{
				rot.z -= deltaTime * speed;
			}

			var scale = transform.Scale;
			if (Input.GetKeyDown(KeyCode.Z))
			{
				scale += deltaTime * 0.5f;
			}
			if (Input.GetKeyDown(KeyCode.C))
			{
				scale -= deltaTime * 0.5f;
			}

			transform.Position = pos;
			transform.Rotation = rot;
			transform.Scale = scale;
		}
	}
}
