#if !defined(AFTERGLOW_H)

#include "Core/Types.h"

struct GameMemory
{
    bool32 Initialized;

    uint64 PermanentDataSize;
    void* PermanentData;

    uint64 TransientDataSize;
    void* TransientData;
};

static void GameUpdateAndRender(GameMemory* gameMemory);

#define AFTERGLOW_H
#endif
