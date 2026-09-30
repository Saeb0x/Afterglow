#if !defined(AFTERGLOW_GPU_H)
#define AFTERGLOW_GPU_H

#include <SSTL/Core/Types.h>
#include <SSTL/Core/String.h>

struct GpuCaps
{
    char8 Name[128];
    uint64 VideoMemory;
    uint32 MaxTextureSize;
    bool Tearing;
};

struct GpuBuffer
{
    void* Object;
};

struct GpuTexture
{
    void* Object;
};

enum class GpuBufferType : uint8
{
    Vertex,
    Index,
    Constant
};

enum class GpuUsage : uint8
{
    Immutable, // Data given at creation, never changes
    Dynamic // Rewritten by the CPU with GpuMapBuffer
};

struct GpuBufferDesc
{
    GpuBufferType Type;
    GpuUsage Usage;
    uint32 Size;
    const void* Data;
    StringView8 DebugName;
};

enum class GpuFormat : uint8
{
    RGBA8,
    R8
};

struct GpuTextureDesc
{
    uint32 Width, Height;
    GpuFormat Format;
    const void* Data; // Rows top to bottom, no padding
    StringView8 DebugName;
};

struct GpuStats
{
    uint32 Buffers;
    uint32 Textures;
};

void GpuGetCaps(GpuCaps* caps);

GpuBuffer GpuCreateBuffer(const GpuBufferDesc* desc);
void GpuDestroyBuffer(GpuBuffer buffer);
void* GpuMapBuffer(GpuBuffer buffer);
void GpuUnmapBuffer(GpuBuffer buffer);

GpuTexture GpuCreateTexture(const GpuTextureDesc* desc);
void GpuDestroyTexture(GpuTexture texture);

void GpuGetStats(GpuStats* stats);

void GpuBeginMarker(StringView8 name);
void GpuEndMarker();

#endif
