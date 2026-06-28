#if !defined(TEXT_H)

#include "Core/Types.h"
#include "Assets/FontFormat.h"
#include "RenderCommands.h"

struct Font
{
    uint32 TextureHandle;
    int32 AtlasWidth, AtlasHeight, LineHeight;
    FontGlyph Glyphs[256];
};

static int32 TextWidth(Font* font, const char* text)
{
    int32 width = 0;

    for(const char* c = text; *c; ++c)
    {
        FontGlyph* glyph = &font->Glyphs[(uint8)*c];
        width += glyph->XAdvance;
    }

    return(width);
}

static void PushText(RenderCommands* commands, Font* font, real32 x, real32 y, const char* text, uint32 color)
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

            PushTexturedQuad(commands, cursorX + (real32)glyph->XOffset, y + (real32)glyph->YOffset, (real32)glyph->Width, (real32)glyph->Height, u0, v0, u1, v1, font->TextureHandle, color);
        }

        cursorX += (real32)glyph->XAdvance;
    }
}

#define TEXT_H
#endif
