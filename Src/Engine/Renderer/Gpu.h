#if !defined(AFTERGLOW_GPU_H)
#define AFTERGLOW_GPU_H

#include <SSTL/Core/Types.h>
#include <SSTL/Core/String.h>

struct GpuCaps
{
    char8 Name[128];
    uint64 VideoMemory; // Dedicated, in bytes
    uint32 MaxTextureSize;
    bool Tearing;
};

void GpuGetCaps(GpuCaps* caps);

void GpuBeginMarker(StringView8 name);
void GpuEndMarker();

#endif
