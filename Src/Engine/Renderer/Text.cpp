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
            RendererQuad quad = {};
            quad.X = penX + glyph->OffsetX * scale;
            quad.Y = baseline + glyph->OffsetY * scale;
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
