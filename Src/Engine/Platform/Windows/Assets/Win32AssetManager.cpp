#include "Engine/Assets/AssetManager.h"
#include "Engine/Platform/Windows/Font/Win32Font.h"
#include "Engine/Platform/Windows/Texture/Win32Texture.h"

void AssetManagerInit(AssetManager* assetManager, void* platformData)
{
    assetManager->PlatformData = platformData;
}

bool32 AssetManagerLoadFont(AssetManager* assetManager, sstl::Arena* transient, Font* font, const char* path)
{
    D3D11RendererState* renderer = (D3D11RendererState*)assetManager->PlatformData;
    return(FontLoad(renderer, transient, font, path));
}

bool32 AssetManagerLoadTexture(AssetManager* assetManager, sstl::Arena* transient, Texture* texture, const char* path)
{
    D3D11RendererState* renderer = (D3D11RendererState*)assetManager->PlatformData;
    return(TextureLoad(renderer, transient, texture, path));
}
