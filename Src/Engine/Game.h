#if !defined(AFTERGLOW_GAME_H)
#define AFTERGLOW_GAME_H

#include <SSTL/Core/Types.h>
#include <SSTL/Memory/StackAllocator.h>

// NOTE(saeb): Only set flags here, never acquire resources (nothing tears this down).
void GameConfigure();

bool GameInit(StackAllocator* allocator);
void GameUpdate(StackAllocator* allocator, real64 deltaTime);
void GameShutdown(StackAllocator* allocator);

#endif
