#if !defined(AFTERGLOW_GPU_H)
#define AFTERGLOW_GPU_H

#include <SSTL/Core/Types.h>
#include <SSTL/Core/String.h>
#include <SSTL/Memory/StackAllocator.h>

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

struct GpuTexture
{
    void* Object;
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

struct GpuPipeline
{
    void* Object;
};

enum class GpuVertexFormat : uint8
{
    Float,
    Float2,
    Float3,
    Float4
};

struct GpuVertexAttribute
{
    uint32 Location; // Which shader input
    GpuVertexFormat Format;
    uint32 Offset; // Bytes into the vertex
};

enum class GpuBlend : uint8
{
    Opaque,
    Premultiplied, // Colours carry their alpha: result = source + destination * (1 - source alpha)
    Additive // result = source + destination
};

enum class GpuCull : uint8
{
    None,
    Back
};

enum class GpuDepth : uint8
{
    Off, // Ignores depth: draws over whatever is there and leaves no depth behind (2D, UI)
    Test, // Hidden behind nearer things, but leaves no depth behind (see-through 3D: glass, water)
    TestWrite // Hidden behind nearer things, and hides farther things drawn after it (solid 3D)
};

enum class GpuPrimitive : uint8
{
    Triangles,
    Lines
};

#define GPU_MAX_VERTEX_ATTRIBUTES 8

struct GpuPipelineDesc
{
    const void* VertexShader;
    usize VertexShaderSize;
    const void* PixelShader;
    usize PixelShaderSize;
    GpuVertexAttribute Attributes[GPU_MAX_VERTEX_ATTRIBUTES];
    uint32 AttributeCount;
    GpuBlend Blend;
    GpuCull Cull;
    GpuDepth Depth;
    GpuPrimitive Primitive;
    StringView8 DebugName;
};

enum class GpuSampler : uint8
{
    LinearClamp,
    LinearWrap,
    PointClamp,
    PointWrap,

    Count
};

enum class GpuIndexFormat : uint8
{
    U16,
    U32
};

struct GpuStats
{
    uint32 Buffers;
    uint32 Textures;
    uint32 Pipelines;
};

void GpuGetCaps(GpuCaps* caps);

GpuBuffer GpuCreateBuffer(const GpuBufferDesc* desc);
void GpuDestroyBuffer(GpuBuffer buffer);
void* GpuMapBuffer(GpuBuffer buffer);
void GpuUnmapBuffer(GpuBuffer buffer);

GpuTexture GpuCreateTexture(const GpuTextureDesc* desc);
void GpuDestroyTexture(GpuTexture texture);

// NOTE(saeb): The pipeline's own record lives in the allocator's Lower heap until that frame is released; GpuDestroyPipeline releases its GPU objects.
GpuPipeline GpuCreatePipeline(StackAllocator* allocator, const GpuPipelineDesc* desc);
void GpuDestroyPipeline(GpuPipeline pipeline);

void GpuSetPipeline(GpuPipeline pipeline);
void GpuSetVertexBuffer(GpuBuffer buffer, uint32 stride);
void GpuSetIndexBuffer(GpuBuffer buffer, GpuIndexFormat format);
void GpuSetConstantBuffer(uint32 slot, GpuBuffer buffer); // Vertex and pixel stages
void GpuSetTexture(uint32 slot, GpuTexture texture, GpuSampler sampler);
void GpuDraw(uint32 vertexCount, uint32 firstVertex);
void GpuDrawIndexed(uint32 indexCount, uint32 firstIndex);

void GpuGetStats(GpuStats* stats);

void GpuBeginMarker(StringView8 name);
void GpuEndMarker();
bool GpuMarkersEnabled();

#endif
