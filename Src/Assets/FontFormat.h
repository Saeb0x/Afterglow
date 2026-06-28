#if !defined(FONT_FORMAT_H)

#include "Core/Types.h"

#define FONT_IDENTIFIER { 'A', 'G', 'F', 'T' }
#define FONT_VERSION 1

struct FontFileHeader
{
    char Identifier[4];
    uint32 Version;
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

#define FONT_FORMAT_H
#endif
