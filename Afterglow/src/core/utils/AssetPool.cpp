#include "AssetPool.h"

namespace Afterglow
{
	namespace Core
	{
		namespace Utils
		{
			AssetPool::AssetPool() {}
			AssetPool::~AssetPool() 
            {
                m_Shaders.clear();
                m_Textures.clear();
            }

            AssetPool& AssetPool::GetInstance()
            {
                static AssetPool assetPoolInstance;
                return assetPoolInstance;
            }

            std::shared_ptr<Graphics::Shader> AssetPool::GetShader(const std::string& vertexResourceName, const std::string& fragmentResourceName)
            {
                // Concatenate the vertex and fragment resource names to form a single key
                std::string shaderKey = vertexResourceName + fragmentResourceName;

                auto it = m_Shaders.find(shaderKey);
                if (it != m_Shaders.end())
                {
                    // Shader already exists, return it
                    return it->second;
                }

                std::shared_ptr<Graphics::Shader> shader = std::make_shared<Graphics::Shader>(vertexResourceName, fragmentResourceName);
                m_Shaders[shaderKey] = shader;
                
                return m_Shaders[shaderKey];
            }

            std::shared_ptr<Graphics::Texture> AssetPool::GetTexture(const std::string& textureResourceName)
            {
                auto it = m_Textures.find(textureResourceName);
                if (it != m_Textures.end())
                {
                    return it->second;
                }

                std::shared_ptr<Graphics::Texture> texture = std::make_shared<Graphics::Texture>(textureResourceName);
                m_Textures[textureResourceName] = texture;

                return m_Textures[textureResourceName];
            }
		}
	}
}