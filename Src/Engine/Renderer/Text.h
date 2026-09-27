#if !defined(AFTERGLOW_TEXT_H)
#define AFTERGLOW_TEXT_H

#include "Renderer.h"

#include <SSTL/Core/Types.h>
#include <SSTL/Core/String.h>

// NOTE(saeb): One glyph, ready to draw: where it sits in the atlas (UVs) and how it's placed relative to the pen on the baseline. Sizes are in pixels at the font's PixelHeight and are scaled when drawing.
struct TextGlyph
{
    uint32 Codepoint;
    real32 U0, V0, U1, V1;
    real32 OffsetX, OffsetY; // From the pen on the baseline to the glyph's top-left
    real32 Width, Height; // 0 x 0 for glyphs with no ink, such as space
    real32 Advance; // How far the pen moves after this glyph
};

// NOTE(saeb): A loaded font (see AssetLoadFont): one signed distance field atlas texture plus the glyph table.
struct Font
{
    RendererTexture Atlas;
    const TextGlyph* Glyphs; // Base address of the glyph array in the Lower heap. Sorted by codepoint
    uint32 GlyphCount;
    uint32 FallbackGlyph; // Index drawn for characters the font doesn't have ('?' when the font has one)
    real32 PixelHeight; // The size the atlas was generated at; TextDraw's size is relative to it
    real32 Ascent, Descent; // Above and below the baseline, both positive
    real32 LineHeight; // Baseline to baseline
    real32 SolidU, SolidV; // A point in the atlas that's fully inside; TextDrawRect samples it
};

// NOTE(saeb): Engine startup only: the signed distance field pipeline every font draws with.
void TextSetPipeline(RendererPipeline pipeline);

// NOTE(saeb): Draws UTF-8 text with its top-left at (x, y). size is the height of a line's letters (ascent plus descent), in the same units as x and y, so text follows the design area like everything else. '\n' starts a new line; characters the font doesn't have draw as its fallback glyph.
void TextDraw(const Font* font, real32 x, real32 y, real32 size, real32 r, real32 g, real32 b, real32 a, StringView8 text);

// NOTE(saeb): The size TextDraw would cover: the widest line, and every line's height (the first line's letters plus a line height for each line after it).
void TextMeasure(const Font* font, real32 size, StringView8 text, real32* width, real32* height);

// NOTE(saeb): A solid rect drawn with this font's atlas and the text pipeline, so it batches with the font's text into the same draw call. UI built from rects and text then costs one draw call per font instead of one per switch between them.
void TextDrawRect(const Font* font, real32 x, real32 y, real32 width, real32 height, real32 r, real32 g, real32 b, real32 a);

#endif
