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

		static std::shared_ptr<Shader> Create(const std::filesystem::path& shadersFilePath);
		static std::shared_ptr<Shader> Create(const std::string& debugName, const std::string& vertexSource, const std::string& fragmentSource);
	};

	class ShaderLibrary
	{
	public:
		static ShaderLibrary& GetInstance();
		
		void Add(const std::string& shaderName, const std::shared_ptr<Shader>& shader);
		std::shared_ptr<Shader> Load(const std::filesystem::path& shadersFilePath, const std::string& shaderName = "");

		std::shared_ptr<Shader> Get(const std::string& shaderName) const;
		bool Exists(const std::string& shaderName) const;

	private:
		ShaderLibrary() = default;
		~ShaderLibrary() = default;
		ShaderLibrary(const ShaderLibrary&);
		const ShaderLibrary& operator=(const ShaderLibrary&);

	private:
		std::unordered_map<std::string, std::shared_ptr<Shader>> m_Shaders;
	};
}