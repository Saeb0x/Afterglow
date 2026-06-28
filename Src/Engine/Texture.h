#if !defined(TEXTURE_H)

#include "Core/Types.h"
#include "RenderCommands.h"

struct Texture
{
    uint32 TextureHandle;
    int32 Width;
    int32 Height;
};

static void PushSprite(RenderCommands* commands, Texture* texture, real32 x, real32 y, uint32 color)
{
    PushTexturedQuad(commands, x, y, (real32)texture->Width, (real32)texture->Height, 0.0f, 0.0f, 1.0f, 1.0f, texture->TextureHandle, color);
}

#define TEXTURE_H
#endif
