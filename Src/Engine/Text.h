#if !defined(AFTERGLOW_TEXT_H)
#define AFTERGLOW_TEXT_H

#include "Engine/Types.h"
#include "FontFormat.h"
#include "RenderCommands.h"
#include "AssetState.h"

struct Font
{
    AssetState State;
    uint32 TextureHandle;
    int32 AtlasWidth, AtlasHeight, LineHeight;
    FontGlyph Glyphs[256];
};

static real32 TextWidth(Font* font, const char* text, real32 scale)
{
    real32 width = 0.0f;

    for(const char* c = text; *c; ++c)
    {
        FontGlyph* glyph = &font->Glyphs[(uint8)*c];
        width += (real32)glyph->XAdvance * scale;
    }

    return(width);
}

static void PushText(RenderCommands* commands, Font* font, real32 x, real32 y, const char* text, uint32 color, real32 scale)
{
    real32 cursorX = x;

    for(const char* c = text; *c; ++c)
    {
        FontGlyph* glyph = &font->Glyphs[(uint8)*c];

        if(glyph->Width > 0 && glyph->Height > 0)
        {
            real32 u0 = (real32)glyph->X / (real32)font->AtlasWidth;
            real32 v0 = (real32)glyph->Y / (real32)font->AtlasHeight;
            real32 u1 = (real32)(glyph->X + glyph->Width) / (real32)font->AtlasWidth;
            real32 v1 = (real32)(glyph->Y + glyph->Height) / (real32)font->AtlasHeight;

            PushTexturedQuad(commands, cursorX + (real32)glyph->XOffset * scale, y + (real32)glyph->YOffset * scale, (real32)glyph->Width * scale, (real32)glyph->Height * scale, u0, v0, u1, v1, font->TextureHandle, color);
        }

        cursorX += (real32)glyph->XAdvance * scale;
    }
}

#endif
