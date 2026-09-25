#if !defined(AFTERGLOW_RENDERER_H)
#define AFTERGLOW_RENDERER_H

#include <SSTL/Core/Types.h>
#include <SSTL/Core/String.h>

enum RendererFlags : uint32
{
    RendererFlags_None,
    RendererFlags_VSync = 1 << 0
};

using RendererTexture = uint32; // 0 = white texture
using RendererPipeline = uint32; // 0 = default quad pipeline

struct RendererQuad
{
    real32 X, Y, Width, Height; // Pixels, top-left origin
    real32 U0, V0, U1, V1; // Texture Coordinates, (U0, V0) = top-left
    real32 R, G, B, A; // Color
    RendererTexture Texture;
    RendererPipeline Pipeline;
};

void RendererSetFlags(uint32 rendererFlags);
void RendererPushQuad(const RendererQuad* quad);

// NOTE(saeb): Pixels are RGBA8, premultiplied, rows top to bottom. Returns 0 (the white texture) on failure. debugName shows up in RenderDoc / PIX and debug-layer messages; it's copied, and an empty one is fine.
RendererTexture RendererCreateTexture(uint32 width, uint32 height, const uint8* pixels, StringView8 debugName);

// NOTE(saeb): Bytecode is a compiled pixel shader (DXBC for D3D11) that (for now) takes the default quad vertex shader's outputs. Returns 0 (the default pipeline) on failure.
RendererPipeline RendererCreatePipeline(const uint8* pixelBytecode, usize size, StringView8 debugName);

// NOTE(saeb): Engine startup only, called once after the renderer is initialized: creates the shared quad vertex shader and input layout, and pipeline 0's pixel shader. Until it succeeds, no quads draw. All or nothing; returns false on failure or if already set.
bool RendererSetDefaultPipeline(const uint8* vertexBytecode, usize vertexSize, const uint8* pixelBytecode, usize pixelSize);

#endif
