#if !defined(AFTERGLOW_RENDERER3DINTERNAL_H)
#define AFTERGLOW_RENDERER3DINTERNAL_H

#include <SSTL/Core/Types.h>
#include <SSTL/Memory/StackAllocator.h>

#include "Engine/Renderer/Internal/RendererInternal.h"

bool Renderer3DInit(StackAllocator* allocator, RendererInitError* error);
void Renderer3DShutdown();

// NOTE(saeb): Draws this frame's meshes when draw is true, then resets for the next frame either way.
void Renderer3DEndFrame(bool draw);

#endif
