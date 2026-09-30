#if !defined(AFTERGLOW_RENDERER2DINTERNAL_H)
#define AFTERGLOW_RENDERER2DINTERNAL_H

#include <SSTL/Core/Types.h>
#include <SSTL/Memory/StackAllocator.h>

bool Renderer2DInit(StackAllocator* allocator);
void Renderer2DShutdown();

// NOTE(saeb): Draws this frame's quads when draw is true, then resets for the next frame either way.
void Renderer2DEndFrame(bool draw);

#endif
