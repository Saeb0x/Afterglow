#if !defined(AFTERGLOW_RENDERERINTERNAL_H)
#define AFTERGLOW_RENDERERINTERNAL_H

#include <SSTL/Core/Types.h>
#include <SSTL/Memory/StackAllocator.h>

// NOTE(saeb): After GpuInit; initializes every rendering system.
bool RendererInit(StackAllocator* allocator);
void RendererShutdown();

void RendererBeginFrame(int32 width, int32 height);
void RendererEndFrame();

#endif
