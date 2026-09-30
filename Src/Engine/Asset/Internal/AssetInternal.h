#if !defined(AFTERGLOW_ASSETINTERNAL_H)
#define AFTERGLOW_ASSETINTERNAL_H

#include "Engine/Asset/Asset.h"

#include <SSTL/Core/Types.h>
#include <SSTL/Core/String.h>
#include <SSTL/Memory/StackAllocator.h>

struct AssetShader
{
    const uint8* Vertex;
    usize VertexSize;
    const uint8* Pixel;
    usize PixelSize;
};

// NOTE(saeb): Reads and validates a cooked .aga shader without creating anything. The bytes live in the allocator's Upper heap: take an Upper frame before, and release it once they're used.
AssetLoadResult AssetReadShader(StackAllocator* allocator, StringView8 path, AssetShader* shader);

#endif
