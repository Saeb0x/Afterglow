#if !defined(AFTERGLOW_RENDER_H)
#define AFTERGLOW_RENDER_H

#include <SSTL/Core/Types.h>

struct Color
{
    real32 R;
    real32 G;
    real32 B;
    real32 A;
};

struct Quad
{
    int32 Position[2];
    int32 Size[2];
    Color Color;
};

void RenderDrawQuad(Quad quad);

#endif
