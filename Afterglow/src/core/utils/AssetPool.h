#pragma once

#include "../graphics/Shader.h"
#include "../graphics/Texture.h"

#include <memory>

namespace Afterglow
{
	namespace Core
	{
		namespace Utils
		{
            class AssetPool
            {
            public:

                static AssetPool& GetInstance();

                std::shared_ptr<Graphics::Shader> GetShader(const std::string& vertexResourceName, const std::string& fragmentResourceName);
                std::shared_ptr<Graphics::Texture> GetTexture(const std::string& textureResourceName);

                inline const std::unordered_map<std::string, std::shared_ptr<Graphics::Shader>>& GetShaders() const { return m_Shaders; }
                inline const std::unordered_map<std::string, std::shared_ptr<Graphics::Texture>>& GetTextures() const { return m_Textures; }
            private:
                AssetPool();
                ~AssetPool();
                AssetPool(const AssetPool&);
                const AssetPool& operator=(const AssetPool&);
            private:
                std::unordered_map<std::string, std::shared_ptr<Graphics::Shader>> m_Shaders;
                std::unordered_map<std::string, std::shared_ptr<Graphics::Texture>> m_Textures;
            };
		}
	}
}



