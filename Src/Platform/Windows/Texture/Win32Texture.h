#if !defined(WIN32TEXTURE_H)

#include "Core/Types.h"
#include "Core/MemoryArena.h"
#include "Engine/Texture.h"

struct D3D11RendererState;

bool32 TextureLoad(D3D11RendererState* renderer, MemoryArena* transient, Texture* texture, const char* path);

#define WIN32TEXTURE_H
#endif
