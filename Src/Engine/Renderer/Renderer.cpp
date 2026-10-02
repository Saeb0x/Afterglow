#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Internal/RendererInternal.h"
#include "Engine/Renderer/Internal/Renderer2DInternal.h"
#include "Engine/Renderer/Internal/Renderer3DInternal.h"
#include "Engine/Renderer/Gpu.h"
#include "Engine/Renderer/Internal/GpuInternal.h"

// NOTE(saeb): The frame: one pass into the back buffer that every rendering system draws into, then present. Systems drawn later land on top.
struct Renderer
{
    bool FrameActive;
    uint32 Flags;
};
static Renderer RendererData;

bool RendererInit(StackAllocator* allocator, RendererInitError* error)
{
    return(Renderer2DInit(allocator, error) && Renderer3DInit(allocator, error));
}

void RendererShutdown()
{
    Renderer3DShutdown();
    Renderer2DShutdown();
}

void RendererBeginFrame(int32 width, int32 height)
{
    RendererData.FrameActive = GpuBeginFrame(width, height);
    if(RendererData.FrameActive)
    {
        GpuPassDesc pass = { { 0.529f, 0.808f, 0.922f, 1.0f } };
        GpuBeginPass(&pass);
    }
}

void RendererEndFrame()
{
    // NOTE(saeb): 3D first, so 2D (UI, text) lands on top of it.
    Renderer3DEndFrame(RendererData.FrameActive);
    Renderer2DEndFrame(RendererData.FrameActive);

    if(RendererData.FrameActive)
    {
        GpuEndPass();
    }

    GpuPresent((RendererData.Flags & RendererFlags_VSync) != 0);
}

void RendererSetFlags(uint32 rendererFlags)
{
    RendererData.Flags = rendererFlags;
}
