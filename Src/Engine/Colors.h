#if !defined(AFTERGLOW_COLORS_H)
#define AFTERGLOW_COLORS_H

#include "Engine/Types.h"

static uint32 PackColor(uint8 r, uint8 g, uint8 b, uint8 a)
{
    return ((uint32)a << 24) | ((uint32)b << 16) | ((uint32)g << 8) | (uint32)r;
}

#define BLACK PackColor(0, 0, 0, 255)
#define WHITE PackColor(255, 255, 255, 255)
#define SKY_BLUE PackColor(138, 187, 214, 255)

#endif
