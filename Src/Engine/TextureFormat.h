#if !defined(TEXTURE_FORMAT_H)

#include "Core/Types.h"
#include "AssetFormat.h"

#define TEXTURE_IDENTIFIER { 'A', 'G', 'T', 'X' }
#define TEXTURE_VERSION 1

struct TextureFileHeader
{
    AssetHeader Header;

    int32 Width;
    int32 Height;
};

// NOTE(saeb): Followed in the file by: Width * Height * 4 raw bytes (RGBA8).

#define TEXTURE_FORMAT_H
#endif
