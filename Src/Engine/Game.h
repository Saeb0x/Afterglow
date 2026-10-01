#if !defined(AFTERGLOW_GAME_H)
#define AFTERGLOW_GAME_H

#include <SSTL/Core/Types.h>
#include <SSTL/Memory/StackAllocator.h>

struct GameTime
{
    real64 Delta; // Real seconds since the previous frame, unclamped: long frames arrive as they are
    real64 Total; // Seconds since startup, not counting time minimized
    uint64 Frame; // 0 on the first frame

    real64 Step; // Seconds per fixed update, always the same: use this in GameFixedUpdate, never Delta
    uint64 StepIndex; // In GameFixedUpdate: which step this is since startup, 0 first
    uint32 Steps; // Fixed updates run this frame: 0 at high refresh rates, up to the cap after a long frame
    real64 Alpha; // In GameUpdate: how far between the last two fixed updates, 0 up to 1
};

// NOTE(saeb): Only set flags here, never acquire resources (nothing tears this down).
void GameConfigure();

bool GameInit(StackAllocator* allocator);
void GameFixedUpdate(StackAllocator* allocator, const GameTime* time);
void GameUpdate(StackAllocator* allocator, const GameTime* time);
void GameShutdown(StackAllocator* allocator);

#endif
