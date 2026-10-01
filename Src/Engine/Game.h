#if !defined(AFTERGLOW_GAME_H)
#define AFTERGLOW_GAME_H

#include <SSTL/Core/Types.h>
#include <SSTL/Memory/StackAllocator.h>

struct GameTime
{
    real64 Delta; // Real seconds since the previous frame, unclamped: long frames arrive as they are
    real64 Total; // Seconds since startup, not counting time minimized
    uint64 Frame; // 0 on the first frame
};

// NOTE(saeb): Only set flags here, never acquire resources (nothing tears this down).
void GameConfigure();

bool GameInit(StackAllocator* allocator);
void GameUpdate(StackAllocator* allocator, const GameTime* time);
void GameShutdown(StackAllocator* allocator);

#endif
