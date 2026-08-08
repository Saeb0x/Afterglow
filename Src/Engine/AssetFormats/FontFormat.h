#if !defined(AFTERGLOW_FONT_FORMAT_H)
#define AFTERGLOW_FONT_FORMAT_H

#include "Engine/Types.h"
#include "AssetFormat.h"

#define FONT_IDENTIFIER { 'A', 'G', 'F', 'T' }
#define FONT_VERSION 1

struct FontFileHeader
{
    AssetHeader Header;

    int32 AtlasWidth;
    int32 AtlasHeight;
    int32 LineHeight;
    uint32 GlyphCount;
};

struct FontGlyph
{
    int32 X, Y;
    int32 Width, Height;
    int32 XOffset, YOffset;
    int32 XAdvance;
};

// NOTE(saeb): Followed in the file by: AtlasWidth * AtlasHeight raw bytes (single-channel alpha).

#endif
