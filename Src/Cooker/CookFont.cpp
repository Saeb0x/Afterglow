#include "Cooker/Cooker.h"

#include "Engine/Asset/Internal/AssetFormat.h"

#include <string.h>

// NOTE(saeb): stb_truetype does no safety checks on the font file, which is fine here: it only ever runs in the cooker, on fonts the project chose, never on anything a player supplies.
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define COOK_FONT_PIXEL_HEIGHT 48.0f // The size the distance fields are generated at; they stay crisp well above and below it
#define COOK_FONT_PADDING 6 // Pixels of distance stored around each glyph, which is also the furthest the shader can see from an edge
#define COOK_FONT_FIRST_CODEPOINT 32 // Printable ASCII: space ...
#define COOK_FONT_LAST_CODEPOINT 126 // ... through '~'
#define COOK_FONT_MAX_GLYPHS (COOK_FONT_LAST_CODEPOINT - COOK_FONT_FIRST_CODEPOINT + 1)
#define COOK_FONT_SOLID_SIZE 4 // Side of the solid block; linear filtering at its centre only reads the middle texels, so the edges give slack

struct CookGlyph
{
    uint32 Codepoint;
    unsigned char* Distances; // From stb_truetype (malloc); null for glyphs with no ink
    int Width, Height, OffsetX, OffsetY;
    real32 Advance;
};

static void CookFontFreeGlyphs(stbtt_fontinfo* info, CookGlyph* glyphs, uint32 count)
{
    for(uint32 index = 0; index < count; ++index)
    {
        if(glyphs[index].Distances)
        {
            stbtt_FreeSDF(glyphs[index].Distances, info->userdata);
            glyphs[index].Distances = nullptr;
        }
    }
}

static bool CookFontWrite(CookerContext* context, StringView8 sourcePath, StringView8 outputPath, stbtt_fontinfo* info, real32 scale, CookGlyph* glyphs, uint32 glyphCount)
{
    // NOTE(saeb): Every glyph gets the same cell, sized to the largest one, laid out as a grid as close to square as possible. Simple and deterministic; a monospaced font wastes almost nothing. One extra cell after the glyphs holds the solid block.
    int cellWidth = COOK_FONT_SOLID_SIZE;
    int cellHeight = COOK_FONT_SOLID_SIZE;
    for(uint32 index = 0; index < glyphCount; ++index)
    {
        cellWidth = (glyphs[index].Width > cellWidth) ? glyphs[index].Width : cellWidth;
        cellHeight = (glyphs[index].Height > cellHeight) ? glyphs[index].Height : cellHeight;
    }

    uint32 cellCount = glyphCount + 1;
    uint32 columns = 1;
    while(columns * columns < cellCount)
    {
        ++columns;
    }

    uint32 rows = (cellCount + columns - 1) / columns;
    uint32 atlasWidth = columns * (uint32)cellWidth;
    uint32 atlasHeight = rows * (uint32)cellHeight;
    if(atlasWidth > AG_ASSET_MAX_TEXTURE_DIMENSION || atlasHeight > AG_ASSET_MAX_TEXTURE_DIMENSION)
    {
        CookerError(sourcePath, "the atlas would be %ux%u; textures are limited to %d on a side", atlasWidth, atlasHeight, AG_ASSET_MAX_TEXTURE_DIMENSION);
        return(false);
    }

    usize glyphBytes = glyphCount * sizeof(AssetGlyph);
    usize atlasBytes = (usize)atlasWidth * atlasHeight;
    usize payloadSize = sizeof(AssetFontHeader) + glyphBytes + atlasBytes;
    usize outputSize = sizeof(AssetHeader) + payloadSize;

    uint8* output = (uint8*)Allocate(context->Memory, Heap::Lower, outputSize, AG_ASSET_ALIGNMENT);
    if(!output)
    {
        CookerError(sourcePath, "out of memory");
        return(false);
    }

    // NOTE(saeb): Zeroed first: padding stays deterministic, and atlas pixels no glyph covers read as "far outside", which draws nothing.
    memset(output, 0, outputSize);

    AssetHeader* header = (AssetHeader*)output;
    header->Magic = AG_ASSET_MAGIC;
    header->Version = AG_ASSET_VERSION;
    header->Type = AssetType::Font;
    header->PayloadSize = payloadSize;

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(info, &ascent, &descent, &lineGap);

    AssetFontHeader* fontHeader = (AssetFontHeader*)(header + 1);
    fontHeader->GlyphCount = glyphCount;
    fontHeader->AtlasWidth = atlasWidth;
    fontHeader->AtlasHeight = atlasHeight;
    fontHeader->PixelHeight = COOK_FONT_PIXEL_HEIGHT;
    fontHeader->Ascent = (real32)ascent * scale;
    fontHeader->Descent = (real32)-descent * scale; // stb_truetype's descent is negative
    fontHeader->LineHeight = (real32)(ascent - descent + lineGap) * scale;
    fontHeader->DistanceRange = (real32)COOK_FONT_PADDING;

    AssetGlyph* glyphTable = (AssetGlyph*)(fontHeader + 1);
    uint8* atlas = (uint8*)(glyphTable + glyphCount);

    for(uint32 index = 0; index < glyphCount; ++index)
    {
        const CookGlyph* source = &glyphs[index];
        uint32 cellX = (index % columns) * (uint32)cellWidth;
        uint32 cellY = (index / columns) * (uint32)cellHeight;

        AssetGlyph* glyph = &glyphTable[index];
        glyph->Codepoint = source->Codepoint;
        glyph->AtlasX = (uint16)cellX;
        glyph->AtlasY = (uint16)cellY;
        glyph->Width = (uint16)source->Width;
        glyph->Height = (uint16)source->Height;
        glyph->OffsetX = (real32)source->OffsetX;
        glyph->OffsetY = (real32)source->OffsetY;
        glyph->Advance = source->Advance;

        for(int row = 0; row < source->Height; ++row)
        {
            memcpy(atlas + (cellY + (uint32)row) * atlasWidth + cellX, source->Distances + row * source->Width, (usize)source->Width);
        }
    }

    // NOTE(saeb): The solid block, in the cell after the last glyph: 255 is as far inside as a distance goes, so the text shader draws it fully covered. Rects sample its centre, which is well inside the block.
    uint32 solidCellX = (glyphCount % columns) * (uint32)cellWidth;
    uint32 solidCellY = (glyphCount / columns) * (uint32)cellHeight;
    for(uint32 row = 0; row < COOK_FONT_SOLID_SIZE; ++row)
    {
        memset(atlas + (solidCellY + row) * atlasWidth + solidCellX, 255, COOK_FONT_SOLID_SIZE);
    }

    fontHeader->SolidX = (uint16)(solidCellX + COOK_FONT_SOLID_SIZE / 2);
    fontHeader->SolidY = (uint16)(solidCellY + COOK_FONT_SOLID_SIZE / 2);

    FileWriteResult writeResult = FileWrite(context->Memory, outputPath, output, outputSize);
    if(writeResult != FileWriteResult::Ok)
    {
        CookerError(outputPath, "couldn't write file (%s)", CookerDescribeWrite(writeResult));
        return(false);
    }

    return(true);
}

bool CookFont(CookerContext* context, StringView8 sourcePath, StringView8 outputPath)
{
    FileContents file;
    FileReadResult readResult = FileRead(context->Memory, Heap::Upper, sourcePath, &file);
    if(readResult != FileReadResult::Ok)
    {
        CookerError(sourcePath, "couldn't read file (%s)", CookerDescribeRead(readResult));
        return(false);
    }

    stbtt_fontinfo info;
    int fontOffset = stbtt_GetFontOffsetForIndex(file.Data, 0);
    if(fontOffset < 0 || !stbtt_InitFont(&info, file.Data, fontOffset))
    {
        CookerError(sourcePath, "not a TrueType font stb_truetype can read");
        return(false);
    }

    real32 scale = stbtt_ScaleForPixelHeight(&info, COOK_FONT_PIXEL_HEIGHT);

    // NOTE(saeb): One distance field per glyph, in codepoint order, which is also the order the engine's binary search needs. Characters the font doesn't have are left out; at runtime they draw as '?'.
    CookGlyph glyphs[COOK_FONT_MAX_GLYPHS] = {};
    uint32 glyphCount = 0;
    for(int codepoint = COOK_FONT_FIRST_CODEPOINT; codepoint <= COOK_FONT_LAST_CODEPOINT; ++codepoint)
    {
        if(codepoint != ' ' && stbtt_FindGlyphIndex(&info, codepoint) == 0)
        {
            continue;
        }

        CookGlyph* glyph = &glyphs[glyphCount++];
        glyph->Codepoint = (uint32)codepoint;

        int advance, leftSideBearing;
        stbtt_GetCodepointHMetrics(&info, codepoint, &advance, &leftSideBearing);
        glyph->Advance = (real32)advance * scale;

        // NOTE(saeb): 128 is the edge, and each pixel of distance moves the value 128 / padding, so the full padding reaches 0 outside or 255 inside.
        glyph->Distances = stbtt_GetCodepointSDF(&info, scale, codepoint, COOK_FONT_PADDING, 128, 128.0f / COOK_FONT_PADDING, &glyph->Width, &glyph->Height, &glyph->OffsetX, &glyph->OffsetY);
        if(!glyph->Distances)
        {
            glyph->Width = 0;
            glyph->Height = 0;
            glyph->OffsetX = 0;
            glyph->OffsetY = 0;
        }
    }

    bool cooked = false;
    if(glyphCount == 0)
    {
        CookerError(sourcePath, "the font has none of the printable ASCII characters");
    }
    else
    {
        cooked = CookFontWrite(context, sourcePath, outputPath, &info, scale, glyphs, glyphCount);
    }

    CookFontFreeGlyphs(&info, glyphs, glyphCount);

    return(cooked);
}
