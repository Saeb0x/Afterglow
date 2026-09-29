#if !defined(AFTERGLOW_RENDERER_H)
#define AFTERGLOW_RENDERER_H

#include <SSTL/Core/Types.h>
#include <SSTL/Core/String.h>

#include "Camera.h"

enum RendererFlags : uint32
{
    RendererFlags_None,
    RendererFlags_VSync = 1 << 0
};

enum class RendererSpace : uint8
{
    World, // Default: world metres, y up, seen through the current camera (RendererSetCamera)
    Screen // Pixels of the window's client area, top-left origin, y down; never scales
};

using RendererTexture = uint32; // 0 = white texture
using RendererPipeline = uint32; // 0 = default quad pipeline

enum class RendererTextureFormat : uint32
{
    RGBA8, // 4 bytes per pixel: premultiplied colour
    R8 // 1 byte per pixel: single values such as signed distance fields; shaders read it as .r
};

struct RendererQuad
{
    real32 X, Y, Width, Height; // The corner with the smallest x and y, and the size: screen pixels (top-left corner, y down) or world metres (bottom-left corner, y up)
    real32 U0, V0, U1, V1; // Texture coordinates; (U0, V0) is the texture's top-left, which stays at the top of the quad in both spaces
    real32 R, G, B, A;
    real32 Rotation; // Radians around the quad's centre, counter-clockwise on screen in both spaces; 0 = axis-aligned
    RendererTexture Texture;
    RendererPipeline Pipeline;
};

struct RendererStats
{
    uint32 Quads;
    uint32 DrawCalls;
    uint32 DroppedQuads;
    uint32 Textures, MaxTextures;
    uint32 Pipelines, MaxPipelines;
    uint32 MaxQuads;
};

void RendererSetFlags(uint32 rendererFlags);
void RendererGetStats(RendererStats* stats);

// NOTE(saeb): Quads pushed after this use the given space until it's changed again; resets to World at the end of every frame.
void RendererSetSpace(RendererSpace space);
RendererSpace RendererGetSpace();

void RendererPushQuad(const RendererQuad* quad);

// NOTE(saeb): Pixels in the given format, rows top to bottom, with no padding between rows. Returns 0 (the white texture) on failure.
RendererTexture RendererCreateTexture(uint32 width, uint32 height, RendererTextureFormat format, const uint8* pixels, StringView8 debugName);

// NOTE(saeb): World-space quads pushed after this use this camera, until another is set; it persists across frames. The camera is copied, so it can change right after. Until the first call, one metre is one pixel, with the world's origin at the window's centre.
void RendererSetCamera(const Camera* camera);

// NOTE(saeb): Bytecode is a compiled pixel shader (DXBC for D3D11) that (for now) takes the default quad vertex shader's outputs. Returns 0 (the default pipeline) on failure.
RendererPipeline RendererCreatePipeline(const uint8* pixelBytecode, usize size, StringView8 debugName);

// NOTE(saeb): Engine startup only, called once after the renderer is initialized: creates the shared quad vertex shader and input layout, and pipeline 0's pixel shader. Until it succeeds, no quads draw. All or nothing; returns false on failure or if already set.
bool RendererSetDefaultPipeline(const uint8* vertexBytecode, usize vertexSize, const uint8* pixelBytecode, usize pixelSize);

#endif
