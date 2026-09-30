#if !defined(AFTERGLOW_GPU_H)
#define AFTERGLOW_GPU_H

#include <SSTL/Core/Types.h>
#include <SSTL/Core/String.h>

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

struct GpuCaps
{
    char8 Name[128];
    uint64 VideoMemory; // Dedicated, in bytes
    uint32 MaxTextureSize;
    bool Tearing;
};

struct GpuPassDesc
{
    real32 ClearColor[4];
};

GpuInitResult GpuInit(const GpuDesc* desc);
void GpuShutdown();
void GpuGetCaps(GpuCaps* caps);

bool GpuBeginFrame(int32 width, int32 height);
void GpuBeginPass(const GpuPassDesc* desc);
void GpuEndPass();
void GpuPresent(bool vsync);

void GpuGetBackBufferSize(int32* width, int32* height);

void GpuBeginMarker(StringView8 name);
void GpuEndMarker();

#endif
