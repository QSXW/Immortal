#include "impch.h"

#include "Shader.h"
#include "Renderer.h"

#include "Immortal/Platform/OpenGL/OpenGLShader.h"

namespace Immortal {

	__forceinline Ref<Shader> Shader::Create(const std::string &filepath)
	{
		return InstantiateGrphicsPrimitive<Shader, OpenGLShader, OpenGLShader, OpenGLShader>(filepath);
	}

	__forceinline Ref<Shader> Shader::Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		return InstantiateGrphicsPrimitive<Shader, OpenGLShader, OpenGLShader, OpenGLShader>(name, vertexSrc, fragmentSrc);
	}

	__forceinline void ShaderMap::Add(const std::string &name, const Ref<Shader> &shader)
	{
		IM_CORE_ASSERT(!Exists(name), "Shader already exists!");
		mShaders[name] = shader;
	}

	__forceinline void ShaderMap::Add(const Ref<Shader> &shader)
	{
		auto name = shader->Name();
		IM_CORE_ASSERT(!Exists(name), "Shader already exists!");
		mShaders[name] = shader;
	}

	Ref<Shader> ShaderMap::Load(const std::string &filepath)
	{
		auto shader = Shader::Create(filepath);
		Add(shader);
		return shader;
	}

	Ref<Shader> ShaderMap::Load(const std::string & name, const std::string & filepath)
	{
		auto shader = Shader::Create(filepath);
		Add(name, shader);
		return shader;
	}

	Ref<Shader> ShaderMap::Get(const std::string &name)
	{
		IM_CORE_ASSERT(Exists(name), "Shader not found!");
		return mShaders[name];
	}

	bool ShaderMap::Exists(const std::string &name) const
	{
		return mShaders.find(name) != mShaders.end();
	}
}
