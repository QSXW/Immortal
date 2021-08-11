#pragma once

#include "ImmortalCore.h"

#include "Render/Shader.h"

#include <glad/glad.h>

namespace Immortal {

	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string &filepath);
		OpenGLShader(const std::string &name, const std::string &vertexSrc, const std::string &fragmentSrc);
		~OpenGLShader();

		void Bind() const override;
		void Unbind() const override;

		void SetUniform(const std::string& name, int value) override;
		void SetUniform(const std::string& name, int* values, uint32_t count) override;
		void SetUniform(const std::string& name, float value) override;
		void SetUniform(const std::string& name, const Vector::Vector2& value) override;
		void SetUniform(const std::string& name, const Vector::Vector3& value) override;
		void SetUniform(const std::string& name, const Vector::Vector4& value) override;
		void SetUniform(const std::string& name, const Vector::mat4& value) override;

		virtual void DispatchCompute(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ);
		
		virtual const uint32_t RendererID() const override { return mRendererID;}
		virtual const std::string &Name() const override { return mName; }

	private:
		void Compile(const std::unordered_map<GLenum, std::string> &source);
		uint32_t CompileShader(int type, const char *src);
		std::string ReadFile(const std::string &filepath);
		std::unordered_map<GLenum, std::string>  Preprocess(const std::string &source);

	private:
		uint32_t mRendererID;
		std::string mName;
		Shader::Type mType = Shader::Type::Fragment;
	};

}
