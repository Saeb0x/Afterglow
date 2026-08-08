#if !defined(AFTERGLOW_ASSET_MANAGER_H)
#define AFTERGLOW_ASSET_MANAGER_H

#include "Engine/Types.h"
#include "Text.h"
#include "Texture.h"

#include "SSTL/Memory.h"

struct AssetManager
{
    void* PlatformData;
};

void AssetManagerInit(AssetManager* assetManager, void* platformData);
bool32 AssetManagerLoadFont(AssetManager* assetManager, sstl::Arena* transient, Font* font, const char* path);
bool32 AssetManagerLoadTexture(AssetManager* assetManager, sstl::Arena* transient, Texture* texture, const char* path);

#endif
