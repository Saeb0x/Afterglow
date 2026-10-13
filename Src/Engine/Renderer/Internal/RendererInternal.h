#if !defined(AFTERGLOW_RENDERERINTERNAL_H)
#define AFTERGLOW_RENDERERINTERNAL_H

#include <Creuse/Core/Types.h>
#include <Creuse/Core/String.h>
#include <Creuse/Memory/StackAllocator.h>

struct RendererInitError
{
    StringView Message;
    StringView Reason;
};

// NOTE(saeb): After GpuInit; initializes every rendering system.
bool RendererInit(StackAllocator* allocator, RendererInitError* error);
void RendererShutdown();

void RendererBeginFrame(int32 width, int32 height);
void RendererEndFrame();

#endif
