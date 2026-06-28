#if !defined(D3D11QUAD_BATCHER_H)

#include "Core/Types.h"
#include "Core/MemoryArena.h"

#include <d3d11.h>

#define MAX_TEXTURE_BATCHES 1024
#define MAX_TEXTURE_HANDLES 256

struct D3D11RendererState;
struct RenderCommands;

enum QuadKind
{
    SOLID,
    TEXTURED,
    GLYPH
};

struct Vertex2D
{
    real32 Position[2];
    real32 UV[2];
    uint32 Color;
};

bool32 D3D11InitQuadBatcher(D3D11RendererState* renderer, MemoryArena* permanent, MemoryArena* transient, uint32 maxQuads);
void D3D11QuadBatcherBegin(int32 width, int32 height);
void D3D11QuadBatcherPushQuad(real32 x, real32 y, real32 w, real32 h, real32 u0, real32 v0, real32 u1, real32 v1, ID3D11ShaderResourceView* texture, uint32 color);
void D3D11QuadBatcherPushGlyphQuad(real32 x, real32 y, real32 w, real32 h, real32 u0, real32 v0, real32 u1, real32 v1, ID3D11ShaderResourceView* atlas, uint32 color);
uint32 D3D11QuadBatcherRegisterTexture(ID3D11ShaderResourceView* texture, QuadKind kind);
void D3D11QuadBatcherSubmitCommands(RenderCommands* commands);
void D3D11QuadBatcherEnd();

#define D3D11QUAD_BATCHER_H
#endif
