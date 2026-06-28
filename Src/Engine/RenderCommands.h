#if !defined(RENDER_COMMANDS_H)

#include "Core/Types.h"

struct RenderCommandQuad
{
    real32 X, Y, Width, Height;
    real32 U0, V0, U1, V1;
    uint32 TextureHandle;
    uint32 Color;
};

struct RenderCommands
{
    RenderCommandQuad* Quads;
    uint32 QuadCount;
    uint32 MaxQuads;
};

static uint32 PackColor(uint8 r, uint8 g, uint8 b, uint8 a)
{
    return ((uint32)a << 24) | ((uint32)b << 16) | ((uint32)g << 8) | (uint32)r;
}

static void PushTexturedQuad(RenderCommands* commands, real32 x, real32 y, real32 width, real32 height, real32 u0, real32 v0, real32 u1, real32 v1, uint32 textureHandle, uint32 color)
{
    Assert(commands->QuadCount < commands->MaxQuads);
    if(commands->QuadCount >= commands->MaxQuads)
    {
        return;
    }

    RenderCommandQuad* quad = &commands->Quads[commands->QuadCount++];
    quad->X = x;
    quad->Y = y;
    quad->Width = width;
    quad->Height = height;
    quad->U0 = u0;
    quad->V0 = v0;
    quad->U1 = u1;
    quad->V1 = v1;
    quad->TextureHandle = textureHandle;
    quad->Color = color;
}

static void PushQuad(RenderCommands* commands, real32 x, real32 y, real32 width, real32 height, uint32 color)
{
    PushTexturedQuad(commands, x, y, width, height, 0.0f, 0.0f, 1.0f, 1.0f, 0, color);
}

#define RENDER_COMMANDS_H
#endif
