#pragma once

#include "../graphics/Shader.h"
#include "../graphics/Texture.h"

#include "../graphics/TextureAtlas.h"

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

                void AddShader(const std::string& vertexResourceName, const std::string& fragmentResourceName);
                std::shared_ptr<Graphics::Shader> GetShader(const std::string& vertexResourceName, const std::string& fragmentResourceName);
               
                void AddTexture(const std::string& textureResourceName);
                std::shared_ptr<Graphics::Texture> GetTexture(const std::string& textureResourceName);
               
                void AddSpriteSheet(const std::string& textureAtlasResourceNamee, int spriteWidth, int spriteHeight, int spritesCount, int spacing);
                std::shared_ptr<Graphics::TextureAtlas> GetSpriteSheet(const std::string& textureAtlasResourceName);

                inline const std::unordered_map<std::string, std::shared_ptr<Graphics::Shader>>& GetShaders() const { return m_Shaders; }
                inline const std::unordered_map<std::string, std::shared_ptr<Graphics::Texture>>& GetTextures() const { return m_Textures; }
                inline const std::unordered_map<std::string, std::shared_ptr<Graphics::TextureAtlas>>& GetTextureAtlases() const { return m_Spritesheets; }
            private:
                AssetPool();
                ~AssetPool();
                AssetPool(const AssetPool&);
                const AssetPool& operator=(const AssetPool&);
            private:
                std::unordered_map<std::string, std::shared_ptr<Graphics::Shader>> m_Shaders;
                std::unordered_map<std::string, std::shared_ptr<Graphics::Texture>> m_Textures;
                std::unordered_map<std::string, std::shared_ptr<Graphics::TextureAtlas>> m_Spritesheets;
            };
		}
	}
}



