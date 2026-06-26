#if !defined(D3D11QUAD_BATCHER_H)

#include "Core/Types.h"
#include "Core/MemoryArena.h"

#include <d3d11.h>

#define MAX_TEXTURE_BATCHES 1024

struct D3D11RendererState;

struct Vertex2D
{
    real32 Position[2];
    real32 UV[2];
    uint32 Color;
};

bool32 D3D11InitQuadBatcher(D3D11RendererState* renderer, MemoryArena* permanentArena, MemoryArena* transientArena, uint32 maxQuads);
void D3D11QuadBatcherBegin(int32 width, int32 height);
void D3D11QuadBatcherPushQuad(real32 x, real32 y, real32 w, real32 h, real32 u0, real32 v0, real32 u1, real32 v1, ID3D11ShaderResourceView* texture, uint32 color);
void D3D11QuadBatcherEnd();

#define D3D11QUAD_BATCHER_H
#endif
