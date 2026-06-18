#include "Afterglow.h"

static void FillScreen(GameOffScreenBitmapBuffer* buffer, int32 xOffset)
{
    uint8* row = (uint8*)buffer->Data;
    for(int32 y = 0; y < buffer->Height; ++y)
    {
        uint32* pixel = (uint32*)row;
        
        for(int32 x = 0; x < buffer->Width; ++x)
        {
            uint8 blue = (uint8)(x + xOffset);
            uint8 green = 0;
            uint8 red = (uint8)y;

            *pixel++ = (red << 16) | (green << 8) | blue;  
        }

        row += buffer->Pitch;
    }
}

static void GameUpdateAndRender(GameOffScreenBitmapBuffer* buffer, int32 xOffset)
{
    FillScreen(buffer, xOffset);
}
