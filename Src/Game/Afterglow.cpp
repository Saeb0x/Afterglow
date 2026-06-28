#include "Afterglow.h"

#include <stdlib.h>

void GameUpdateAndRender(GameContext* context)
{
    for(uint8 i = 0; i < 100; ++i)
    {
        real32 width = (real32)((rand() % 100) + 1);
        real32 height = (real32)((rand() % 100) + 1);
        real32 x = (real32)((rand() % context->ScreenWidth) - width);
        real32 y = (real32)((rand() % context->ScreenHeight) - height);

        uint8 r = (uint8)(rand() % 256);
        uint8 g = (uint8)(rand() % 256);
        uint8 b = (uint8)(rand() % 256);

        PushQuad(context->Render, x, y, width, height, PackColor(r, g, b, 255));
    }
}
