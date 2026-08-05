#if !defined(AFTERGLOW_WIN32TEXTURE_H)
#define AFTERGLOW_WIN32TEXTURE_H

#include "Engine/Types.h"
#include "SSTL/Memory.h"
#include "Engine/Texture.h"

struct D3D11RendererState;

bool32 TextureLoad(D3D11RendererState* renderer, sstl::Arena* transient, Texture* texture, const char* path);

#endif
