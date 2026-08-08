#if !defined(AFTERGLOW_TEXTURE_H)
#define AFTERGLOW_TEXTURE_H

#include "Engine/Types.h"
#include "Engine/Render/RenderCommands.h"
#include "AssetState.h"

struct Texture
{
    AssetState State;
    uint32 TextureHandle;
    int32 Width;
    int32 Height;
};

static void PushSprite(RenderCommands* commands, Texture* texture, real32 x, real32 y, uint32 color)
{
    PushTexturedQuad(commands, x, y, (real32)texture->Width, (real32)texture->Height, 0.0f, 0.0f, 1.0f, 1.0f, texture->TextureHandle, color);
}

#endif
