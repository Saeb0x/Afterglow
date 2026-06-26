#if !defined(RENDER_COMMANDS_H)

#include "Core/Types.h"

struct RenderCommandQuad
{
    real32 X, Y, Width, Height;
    real32 U0, V0, U1, V1;
    uint32 TextureHandle; // 0 = solid (white); non-zero = an engine texture id (later)
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

static void PushQuad(RenderCommands* commands, real32 x, real32 y, real32 width, real32 height, uint32 color)
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
    quad->U0 = 0.0f;
    quad->V0 = 0.0f;
    quad->U1 = 1.0f;
    quad->V1 = 1.0f;
    quad->TextureHandle = 0;
    quad->Color = color;
}

#define RENDER_COMMANDS_H
#endif
