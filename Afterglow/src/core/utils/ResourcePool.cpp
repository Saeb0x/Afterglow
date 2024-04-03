#include "ResourcePool.h"

#include <iostream>

namespace Afterglow
{
	namespace Core
	{
		namespace Utils
		{
            ResourcePool::ResourcePool() {}
            ResourcePool::~ResourcePool()
            {
                m_Shaders.clear();
                m_Textures.clear();
                m_Spritesheets.clear();
            }

            ResourcePool& ResourcePool::GetInstance()
            {
                static ResourcePool ResourcePoolInstance;
                return ResourcePoolInstance;
            }

            void ResourcePool::AddShader(const std::string& vertexResourceName, const std::string& fragmentResourceName)
            {
                // Concatenate the vertex and fragment resource names to form a single key
                std::string shaderKey = vertexResourceName + fragmentResourceName;

                auto it = m_Shaders.find(shaderKey);
                if (it != m_Shaders.end())
                {
                    std::cout << "Added shader to ResourcePool" << std::endl;
                    return;
                }

                std::shared_ptr<Graphics::Shader> shader = std::make_shared<Graphics::Shader>(vertexResourceName, fragmentResourceName);
                m_Shaders[shaderKey] = shader;
            }

            std::shared_ptr<Graphics::Shader> ResourcePool::GetShader(const std::string& vertexResourceName, const std::string& fragmentResourceName)
            {
                std::string shaderKey = vertexResourceName + fragmentResourceName;

                auto it = m_Shaders.find(shaderKey);
                if (it != m_Shaders.end())
                    return it->second; // Shader already exists, return it
                else
                {
                    std::cout << "Shader["<< shaderKey << "]" << " not found in ResourcePool" << std::endl;
                    return nullptr; 
                }
                    
            }

            void ResourcePool::AddTexture(const std::string& textureResourceName)
            {
                auto it = m_Textures.find(textureResourceName);
                if (it != m_Textures.end())
                {
                    std::cout << "Added texture to ResourcePool" << std::endl;
                    return;
                }

                std::shared_ptr<Graphics::Texture> texture = std::make_shared<Graphics::Texture>(textureResourceName);
                m_Textures[textureResourceName] = texture;
            }

            std::shared_ptr<Graphics::Texture> ResourcePool::GetTexture(const std::string& textureResourceName)
            {
                auto it = m_Textures.find(textureResourceName);
                if (it != m_Textures.end())
                    return it->second; // Texture already exists, return it
                else
                {
                    std::cout << "Texture[" << textureResourceName << "]" << " not found in ResourcePool" << std::endl;
                    return nullptr;
                }
                    
            }

            void ResourcePool::AddSpriteSheet(const std::string& textureAtlasResourceName, int spriteWidth, int spriteHeight, int spritesCount, int spacing)
            {
                auto it = m_Spritesheets.find(textureAtlasResourceName);
                if (it != m_Spritesheets.end())
                {
                    std::cout << "Added texture atlas to ResourcePool" << std::endl;
                    return;
                }

                auto itTexture = m_Textures.find(textureAtlasResourceName);
                if (itTexture != m_Textures.end())
                {
                    std::shared_ptr<Graphics::TextureAtlas> spritesheet = std::make_shared<Graphics::TextureAtlas>(GetTexture(textureAtlasResourceName), spriteWidth, spriteHeight, spritesCount, spacing);
                    m_Spritesheets[textureAtlasResourceName] = spritesheet;
                }
                else
                {
                    AddTexture(textureAtlasResourceName);
                    std::shared_ptr<Graphics::TextureAtlas> spritesheet = std::make_shared<Graphics::TextureAtlas>(GetTexture(textureAtlasResourceName), spriteWidth, spriteHeight, spritesCount, spacing);
                    m_Spritesheets[textureAtlasResourceName] = spritesheet;
                }
            }

            std::shared_ptr<Graphics::TextureAtlas> ResourcePool::GetSpriteSheet(const std::string& textureAtlasResourceName)
            {
                auto it = m_Spritesheets.find(textureAtlasResourceName);
                if (it != m_Spritesheets.end())
                    return it->second; // Texture Atlas already exists, return it
                else
                {
                    std::cout << "Texture Atlas[" << textureAtlasResourceName << "]" << " not found in ResourcePool" << std::endl;
                    return nullptr;
                }
            }
		}
	}
}