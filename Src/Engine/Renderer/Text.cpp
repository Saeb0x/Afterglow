#include "Text.h"

static RendererPipeline TextPipeline;

void TextSetPipeline(RendererPipeline pipeline)
{
    TextPipeline = pipeline;
}

// NOTE(saeb): Glyphs are sorted by codepoint, so a binary search; anything missing gets the fallback glyph.
static const TextGlyph* TextFindGlyph(const Font* font, uint32 codepoint)
{
    uint32 low = 0;
    uint32 high = font->GlyphCount;
    while(low < high)
    {
        uint32 middle = low + (high - low) / 2;
        uint32 found = font->Glyphs[middle].Codepoint;

        if(found == codepoint)
        {
            return(&font->Glyphs[middle]);
        }

        if(found < codepoint)
        {
            low = middle + 1;
        }
        else
        {
            high = middle;
        }
    }

    return(&font->Glyphs[font->FallbackGlyph]);
}

// NOTE(saeb): Rounds to the nearest whole number; also correct for negatives, where a plain (int32) cast rounds toward zero.
static real32 TextRoundToPixel(real32 value)
{
    real32 shifted = value + 0.5f;
    int32 whole = (int32)shifted;
    if((real32)whole > shifted)
    {
        --whole;
    }

    return((real32)whole);
}

static bool TextIsUsable(const Font* font, real32 size)
{
    return(font && font->Glyphs && font->GlyphCount > 0 && font->PixelHeight > 0.0f && size > 0.0f);
}

void TextDraw(const Font* font, real32 x, real32 y, real32 size, real32 r, real32 g, real32 b, real32 a, StringView8 text)
{
    if(!TextIsUsable(font, size) || !text.Data)
    {
        return;
    }

    real32 scale = size / font->PixelHeight;
    real32 penX = x;
    real32 baseline = y + font->Ascent * scale;

    // NOTE(saeb): In window space one unit is one pixel, so small text is snapped to the pixel grid: each glyph starts on a whole pixel horizontally, which keeps stems crisp, and each line's baseline is snapped once, so every glyph on the line sits on the same pixel row. Rounding each glyph's top instead would scatter the baseline, since glyph tops sit at different fractions of a pixel. Design space isn't snapped, so moving game text stays smooth.
    bool snap = (RendererGetSpace() == RendererSpace::Window);

    for(usize index = 0; index < text.Length;)
    {
        String8Decoded decoded = String8DecodeNext(text, index);
        index += decoded.Length;

        if(decoded.Codepoint == '\n')
        {
            penX = x;
            baseline += font->LineHeight * scale;
            continue;
        }

        const TextGlyph* glyph = TextFindGlyph(font, decoded.Codepoint);

        // NOTE(saeb): Every glyph shares the atlas and the pipeline, so a whole block of text batches into one draw call.
        if(glyph->Width > 0.0f && glyph->Height > 0.0f)
        {
            real32 glyphX = penX + glyph->OffsetX * scale;
            real32 glyphBaseline = baseline;
            if(snap)
            {
                glyphX = TextRoundToPixel(glyphX);
                glyphBaseline = TextRoundToPixel(baseline);
            }

            RendererQuad quad = {};
            quad.X = glyphX;
            quad.Y = glyphBaseline + glyph->OffsetY * scale;
            quad.Width = glyph->Width * scale;
            quad.Height = glyph->Height * scale;
            quad.U0 = glyph->U0; quad.V0 = glyph->V0;
            quad.U1 = glyph->U1; quad.V1 = glyph->V1;
            quad.R = r; quad.G = g; quad.B = b; quad.A = a;
            quad.Texture = font->Atlas;
            quad.Pipeline = TextPipeline;
            RendererPushQuad(&quad);
        }

        penX += glyph->Advance * scale;
    }
}

void TextDrawRect(const Font* font, real32 x, real32 y, real32 width, real32 height, real32 r, real32 g, real32 b, real32 a)
{
    if(!font)
    {
        return;
    }

    // NOTE(saeb): All four corners sample the same fully-inside point, so the shader's coverage is 1 everywhere and the rect is just its colour.
    RendererQuad quad = {};
    quad.X = x; quad.Y = y; quad.Width = width; quad.Height = height;
    quad.U0 = font->SolidU; quad.V0 = font->SolidV;
    quad.U1 = font->SolidU; quad.V1 = font->SolidV;
    quad.R = r; quad.G = g; quad.B = b; quad.A = a;
    quad.Texture = font->Atlas;
    quad.Pipeline = TextPipeline;
    RendererPushQuad(&quad);
}

void TextMeasure(const Font* font, real32 size, StringView8 text, real32* width, real32* height)
{
    *width = 0.0f;
    *height = 0.0f;

    if(!TextIsUsable(font, size) || !text.Data)
    {
        return;
    }

    real32 scale = size / font->PixelHeight;
    real32 lineWidth = 0.0f;
    uint32 extraLines = 0;

    for(usize index = 0; index < text.Length;)
    {
        String8Decoded decoded = String8DecodeNext(text, index);
        index += decoded.Length;

        if(decoded.Codepoint == '\n')
        {
            ++extraLines;
            lineWidth = 0.0f;
            continue;
        }

        lineWidth += TextFindGlyph(font, decoded.Codepoint)->Advance * scale;
        if(lineWidth > *width)
        {
            *width = lineWidth;
        }
    }

    *height = (font->Ascent + font->Descent) * scale + (real32)extraLines * font->LineHeight * scale;
}
