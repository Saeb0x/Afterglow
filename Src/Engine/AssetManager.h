#if !defined(ASSET_MANAGER_H)

#include "Core/Types.h"
#include "Core/Arena.h"
#include "Engine/Text.h"
#include "Engine/Texture.h"

struct AssetManager
{
    void* PlatformData;
};

void AssetManagerInit(AssetManager* assetManager, void* platformData);
bool32 AssetManagerLoadFont(AssetManager* assetManager, Arena* transient, Font* font, const char* path);
bool32 AssetManagerLoadTexture(AssetManager* assetManager, Arena* transient, Texture* texture, const char* path);

#define ASSET_MANAGER_H
#endif
