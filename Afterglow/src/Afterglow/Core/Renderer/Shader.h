#pragma once

#include <filesystem>

#include <glm/glm.hpp>

namespace Afterglow
{
	class Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual uint32_t GetRendererID() const = 0;

		virtual void SetInt(const std::string& uniform, int num) const = 0;
		virtual void SetFloat(const std::string& uniform, float num) const = 0;
		virtual void SetFloat2(const std::string& uniform, const glm::vec2& vec) const = 0;
		virtual void SetFloat3(const std::string& uniform, const glm::vec3& vec) const = 0;
		virtual void SetFloat4(const std::string& uniform, const glm::vec4& vec) const = 0;
		virtual void SetMat4(const std::string& uniform, const glm::mat4& mat) const = 0;

		static Shader* Create(const std::filesystem::path& shadersFilePath);
		static Shader* Create(const std::string& debugName, const std::string& vertexSource, const std::string& fragmentSource);
	};
}