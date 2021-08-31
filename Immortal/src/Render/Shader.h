#pragma once


#include "ImmortalCore.h"

namespace Immortal {

	class IMMORTAL_API Shader
	{
	public:
		enum class DataType
		{
			None = 0,
			Float,
			Float2,
			Float3,
			Float4,
			Mat3,
			Mat4,
			Int,
			Int2,
			Int3,
			Int4,
			Bool
		};

		enum class Type
		{
			Vertex,
			Fragment,
			Pixel = Fragment,
			Compute
		};

	public:
		virtual ~Shader() = default;

		virtual void Map() const = 0;
		virtual void UnMap() const = 0;
		
		virtual const std::string &Name() const = 0;
		virtual const uint32_t RendererID() const = 0;

		virtual void SetUniform(const std::string& name, int value) = 0;
		virtual void SetUniform(const std::string& name, int* values, uint32_t count) = 0;
		virtual void SetUniform(const std::string& name, float value) = 0;
		virtual void SetUniform(const std::string& name, const Vector::Vector2& value) = 0;
		virtual void SetUniform(const std::string& name, const Vector::Vector3& value) = 0;
		virtual void SetUniform(const std::string& name, const Vector::Vector4& value) = 0;
		virtual void SetUniform(const std::string& name, const Vector::mat4& value) = 0;

		virtual void DispatchCompute(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ) = 0;

		static Ref<Shader> Create(const std::string &filepath);
		static Ref<Shader> Create(const std::string &name, const std::string &vertexSrc, const std::string &fragmentSrc);

	};

	using ShaderDataType = Shader::DataType;

	class IMMORTAL_API ShaderMap
	{
	public:
		void Add(const std::string &name, const Ref<Shader> &shader);
		void Add(const Ref<Shader> &shader);
		
		Ref<Shader> Load(const std::string &filepath);
		Ref<Shader> Load(const std::string &name, const std::string &filepath);
		
		Ref<Shader> Get(const std::string &name);
		__forceinline Ref<Shader> At(const std::string &name)
		{
			return this->Get(name);
		}

		bool Exists(const std::string &name) const;

	private:
		std::unordered_map<std::string, Ref<Shader> > mShaders;
	};
}