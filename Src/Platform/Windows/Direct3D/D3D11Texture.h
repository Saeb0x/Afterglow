#if !defined(D3D11TEXTURE_H)

#include "Core/Types.h"
#include "Core/MemoryArena.h"

#include <d3d11.h>

bool32 D3D11TextureInit();
bool32 D3D11LoadTexture(MemoryArena* transientArena, const wchar_t* path, ID3D11ShaderResourceView** outSRV, int32* outWidth, int32* outHeight);

#define D3D11TEXTURE_H
#endif
