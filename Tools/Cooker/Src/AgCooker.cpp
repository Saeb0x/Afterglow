#include "Assets/FontFormat.h"
#include "Assets/TextureFormat.h"

#define STB_IMAGE_IMPLEMENTATION
#include "Deps/stb_image.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// NOTE(saeb): Parse the signed integer following "key=" within a single line.
static int32 ParseKeyInt(char* lineStart, char* lineEnd, const char* key)
{
    size_t keyLength = strlen(key);

    for(char* c = lineStart; c + keyLength < lineEnd; ++c)
    {
        if(memcmp(c, key, keyLength) == 0 && c[keyLength] == '=')
        {
            char* v = c + keyLength + 1;

            int32 sign = 1;
            if(v < lineEnd && *v == '-')
            {
                sign = -1;
                ++v;
            }

            int32 value = 0;
            while(v < lineEnd && *v >= '0' && *v <= '9')
            {
                value = value * 10 + (*v - '0');
                ++v;
            }

            return(sign * value);
        }
    }

    return(0);
}

static char* ReadEntireFile(const char* path, size_t* outSize)
{
    FILE* file = fopen(path, "rb");
    if(!file)
    {
        return(0);
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = (char*)malloc(size);
    fread(buffer, 1, size, file);
    fclose(file);

    *outSize = (size_t)size;
    return(buffer);
}

static int CookFont(const char* fontPath, const char* pngPath, const char* outPath)
{
    // NOTE(saeb): Decode the atlas with stb_image, keep only the alpha channel.
    int32 imageWidth, imageHeight, imageChannels;

    uint8* pixels = stbi_load(pngPath, &imageWidth, &imageHeight, &imageChannels, 4);
    if(!pixels)
    {
        fprintf(stderr, "Failed to load %s: %s\n", pngPath, stbi_failure_reason());
        return(1);
    }

    uint8* alphaPixels = (uint8*)malloc(imageWidth * imageHeight);
    for(int32 i = 0; i < imageWidth * imageHeight; ++i)
    {
        alphaPixels[i] = pixels[i * 4 + 3]; // Take the A channel of each RGBA texel
    }
    stbi_image_free(pixels);

    // NOTE(saeb): Parse the BMFont .fnt text into our fixed 256-entry glyph table.
    size_t fontSize;
    char* fontText = ReadEntireFile(fontPath, &fontSize);
    if(!fontText)
    {
        fprintf(stderr, "Failed to read %s\n", fontPath);
        return(1);
    }

    FontGlyph glyphs[256] = {};
    int32 lineHeight = 0;

    char* p = fontText;
    char* end = fontText + fontSize;

    while(p < end)
    {
        char* lineStart = p;
        char* lineEnd = p;
        while(lineEnd < end && *lineEnd != '\r' && *lineEnd != '\n')
        {
            ++lineEnd;
        }

        if((lineEnd - lineStart) >= 5 && memcmp(lineStart, "char ", 5) == 0)
        {
            int32 id = ParseKeyInt(lineStart, lineEnd, "id");
            if(id >= 0 && id < 256)
            {
                FontGlyph* glyph = &glyphs[id];
                glyph->X = ParseKeyInt(lineStart, lineEnd, "x");
                glyph->Y = ParseKeyInt(lineStart, lineEnd, "y");
                glyph->Width = ParseKeyInt(lineStart, lineEnd, "width");
                glyph->Height = ParseKeyInt(lineStart, lineEnd, "height");
                glyph->XOffset = ParseKeyInt(lineStart, lineEnd, "xoffset");
                glyph->YOffset = ParseKeyInt(lineStart, lineEnd, "yoffset");
                glyph->XAdvance = ParseKeyInt(lineStart, lineEnd, "xadvance");
            }
        }
        else if((lineEnd - lineStart) >= 7 && memcmp(lineStart, "common ", 7) == 0)
        {
            lineHeight = ParseKeyInt(lineStart, lineEnd, "lineHeight");
        }

        p = lineEnd;
        while(p < end && (*p == '\r' || *p == '\n'))
        {
            ++p;
        }
    }

    free(fontText);

    // NOTE(saeb): Write the .agfont: header, then 256 glyphs, then raw alpha pixels.
    FontFileHeader header = {};
    char fontIdentifier[4] = FONT_IDENTIFIER;
    memcpy(header.Identifier, fontIdentifier, 4);
    header.Version = FONT_VERSION;
    header.AtlasWidth = imageWidth;
    header.AtlasHeight = imageHeight;
    header.LineHeight = lineHeight;
    header.GlyphCount = 256;

    FILE* outFile = fopen(outPath, "wb");
    if(!outFile)
    {
        fprintf(stderr, "Failed to open %s for writing\n", outPath);
        return(1);
    }

    fwrite(&header, sizeof(header), 1, outFile);
    fwrite(glyphs, sizeof(FontGlyph), 256, outFile);
    fwrite(alphaPixels, 1, imageWidth * imageHeight, outFile);

    fclose(outFile);
    free(alphaPixels);

    printf("Cooked %s (%dx%d atlas, lineHeight=%d) -> %s\n", fontPath, imageWidth, imageHeight, lineHeight, outPath);
    return(0);
}

static int CookTexture(const char* pngPath, const char* outPath)
{
    int32 width, height, channels;

    uint8* pixels = stbi_load(pngPath, &width, &height, &channels, 4);
    if(!pixels)
    {
        fprintf(stderr, "Failed to load %s: %s\n", pngPath, stbi_failure_reason());
        return(1);
    }

    TextureFileHeader header = {};
    char textureIdentifier[4] = TEXTURE_IDENTIFIER;
    memcpy(header.Identifier, textureIdentifier, 4);
    header.Version = TEXTURE_VERSION;
    header.Width = width;
    header.Height = height;

    FILE* outFile = fopen(outPath, "wb");
    if(!outFile)
    {
        fprintf(stderr, "Failed to open %s for writing\n", outPath);
        stbi_image_free(pixels);
        return(1);
    }
    
    fwrite(&header, sizeof(header), 1, outFile);
    fwrite(pixels, 1, width * height * 4, outFile);

    fclose(outFile);
    stbi_image_free(pixels);

    printf("Cooked %s (%dx%d RGBA) -> %s\n", pngPath, width, height, outPath);
    return(0);
}

int main(int argc, char** argv)
{
    if(argc < 2)
    {
        fprintf(stderr, "Usage: AgCooker.exe <font|texture> ...\n");
        return(1);
    }

    const char* assetType = argv[1];

    if(strcmp(assetType, "font") == 0)
    {
        if(argc != 5)
        {
            fprintf(stderr, "Usage: AgCooker.exe font <in.fnt> <in.png> <out.agfont>\n");
            return(1);
        }

        return(CookFont(argv[2], argv[3], argv[4]));
    }
    else if(strcmp(assetType, "texture") == 0)
    {
        if(argc != 4)
        {
            fprintf(stderr, "Usage: AgCooker.exe texture <in.png> <out.agtex>\n");
            return(1);
        }

        return(CookTexture(argv[2], argv[3]));
    }

    fprintf(stderr, "Unknown asset type '%s'\n", assetType);
    return(1);
}
