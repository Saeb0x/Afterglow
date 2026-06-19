#if !defined(AFTERGLOW_H)

#include "Core/Types.h"

struct WindowDimensions
{
    int32 Width;
    int32 Height;
};

struct GameOffscreenBitmapBuffer
{
    void* Data;

    int32 Width;
    int32 Height;

    uint8 BytesPerPixel;
    int32 Pitch;
};
static void GameUpdateAndRender(GameOffscreenBitmapBuffer* buffer);

#define AFTERGLOW_H
#endif
