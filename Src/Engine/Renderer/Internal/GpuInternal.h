#if !defined(AFTERGLOW_GPUINTERNAL_H)
#define AFTERGLOW_GPUINTERNAL_H

#include <SSTL/Core/Types.h>

struct GpuDesc
{
    void* WindowHandle;
};

enum class GpuInitResult : uint8
{
    Ok,
    NoGpu, // No hardware GPU found
    Unsupported, // The GPU lacks features needed
    SwapChainFailed
};

struct GpuPassDesc
{
    real32 ClearColor[4];
};

GpuInitResult GpuInit(const GpuDesc* desc);
void GpuShutdown();

bool GpuBeginFrame(int32 width, int32 height);
void GpuBeginPass(const GpuPassDesc* desc);
void GpuEndPass();
void GpuPresent(bool vsync);

void GpuGetBackBufferSize(int32* width, int32* height);

#endif
