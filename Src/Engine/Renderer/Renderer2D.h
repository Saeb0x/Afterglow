#if !defined(AFTERGLOW_RENDERER2D_H)
#define AFTERGLOW_RENDERER2D_H

#include <SSTL/Core/Types.h>
#include <SSTL/Core/String.h>

#include "Engine/Renderer/Camera.h"
#include "Engine/Renderer/Gpu.h"
#include "Engine/Asset/Asset.h"

enum class Renderer2DSpace : uint8
{
    World, // Default: world metres, y up, seen through the current camera (Renderer2DSetCamera)
    Screen // Pixels of the window's client area, top-left origin, y down; never scales
};

using Renderer2DPipeline = uint32; // 0 = default quad pipeline

enum class Renderer2DBlend : uint8
{
    Normal, // Alpha blending: the quad covers what's behind it by its alpha
    Additive // Adds light to what's behind it: glows, sparks; black adds nothing
};

enum class Renderer2DMode : uint8
{
    Sprite, // The texture times the colour; with no texture, the colour alone
    Text // The texture is a signed distance field (a font atlas); draws its edge, crisp at any size
};

struct Renderer2DQuad
{
    real32 X, Y, Width, Height; // The corner with the smallest x and y, and the size: screen pixels (top-left corner, y down) or world metres (bottom-left corner, y up)
    real32 U0, V0, U1, V1; // Texture coordinates; (U0, V0) is the texture's top-left, which stays at the top of the quad in both spaces
    real32 R, G, B, A;
    real32 Rotation; // Radians around the quad's centre, counter-clockwise on screen in both spaces; 0 = axis-aligned
    GpuTexture Texture; // {0} draws solid
    Renderer2DMode Mode;
    Renderer2DPipeline Pipeline;
};

struct Renderer2DStats
{
    uint32 Quads;
    uint32 DrawCalls;
    uint32 DroppedQuads;
    uint32 Pipelines, MaxPipelines;
    uint32 MaxQuads;
};

void Renderer2DGetStats(Renderer2DStats* stats);

// NOTE(saeb): Quads pushed after this use the given space until it's changed again; resets to World at the end of every frame.
void Renderer2DSetSpace(Renderer2DSpace space);
Renderer2DSpace Renderer2DGetSpace();

void Renderer2DPushQuad(const Renderer2DQuad* quad);

// NOTE(saeb): World-space quads pushed after this use this camera, until another is set; it persists across frames. The camera is copied, so it can change right after. Until the first call, one metre is one pixel, with the world's origin at the window's centre.
void Renderer2DSetCamera(const Camera* camera);

// NOTE(saeb): Loads a cooked .aga shader and creates a quad pipeline from it: its PSMain, and its VSMain if it has one (otherwise the quad vertex shader). Every quad pipeline takes the quad vertex and draws triangles, so a custom VSMain must read the quad vertex's inputs (ATTRIB0 to ATTRIB3) and pass on what its PSMain reads. On any failure, *pipeline is 0 (the default pipeline). The file is scratch; the pipeline's record stays in the Lower heap.
AssetLoadResult Renderer2DLoadPipeline(StackAllocator* allocator, StringView8 path, Renderer2DBlend blend, Renderer2DPipeline* pipeline);

#endif
