#include "Engine/Assets/Formats/FontFormat.h"
#include "Engine/Assets/Formats/TextureFormat.h"
#include "Engine/Assets/Formats/ShaderFormat.h"

#define STB_IMAGE_IMPLEMENTATION
#include "Deps/stb_image.h"
#include <windows.h>
#include <d3dcompiler.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static bool32 GetFileWriteTime(const char* path, FILETIME* outTime)
{
    WIN32_FILE_ATTRIBUTE_DATA data;
    if(!GetFileAttributesExA(path, GetFileExInfoStandard, &data))
    {
        return(false);
    }

    *outTime = data.ftLastWriteTime;
    return(true);
}

static bool32 IsUpToDate(const char* outPath, const char** sourcePaths, int sourceCount)
{
    FILETIME outputTime;
    if(!GetFileWriteTime(outPath, &outputTime))
    {
        return(false);
    }

    for(int i = 0; i < sourceCount; ++i)
    {
        FILETIME sourceTime;
        if(!GetFileWriteTime(sourcePaths[i], &sourceTime))
        {
            return(false);
        }

        if(CompareFileTime(&sourceTime, &outputTime) > 0)
        {
            return(false);
        }
    }

    // NOTE(saeb): A rebuilt cooker can change the output format, so treat the cooker itself as an input.
    char cookerPath[MAX_PATH];
    if(GetModuleFileNameA(0, cookerPath, sizeof(cookerPath)))
    {
        FILETIME cookerTime;
        if(GetFileWriteTime(cookerPath, &cookerTime) && CompareFileTime(&cookerTime, &outputTime) > 0)
        {
            return(false);
        }
    }

    return(true);
}

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
    const char* sources[] = { fontPath, pngPath };
    if(IsUpToDate(outPath, sources, 2))
    {
        printf("Up to date: %s\n", outPath);
        return(0);
    }

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

    // NOTE(saeb): Write the .aga: header, then 256 glyphs, then raw alpha pixels.
    FontFileHeader header = {};
    char fontIdentifier[4] = FONT_IDENTIFIER;
    memcpy(header.Header.Identifier, fontIdentifier, 4);
    header.Header.Version = FONT_VERSION;
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
    const char* sources[] = { pngPath };
    if(IsUpToDate(outPath, sources, 1))
    {
        printf("Up to date: %s\n", outPath);
        return(0);
    }

    int32 width, height, channels;

    uint8* pixels = stbi_load(pngPath, &width, &height, &channels, 4);
    if(!pixels)
    {
        fprintf(stderr, "Failed to load %s: %s\n", pngPath, stbi_failure_reason());
        return(1);
    }

    TextureFileHeader header = {};
    char textureIdentifier[4] = TEXTURE_IDENTIFIER;
    memcpy(header.Header.Identifier, textureIdentifier, 4);
    header.Header.Version = TEXTURE_VERSION;
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

static int CookShader(const char* hlslPath, const char* entryPoint, const char* target, const char* outPath)
{
    const char* sources[] = { hlslPath };
    if(IsUpToDate(outPath, sources, 1))
    {
        printf("Up to date: %s\n", outPath);
        return(0);
    }

    FILE* sourceFile = fopen(hlslPath, "rb");
    if(!sourceFile)
    {
        fprintf(stderr, "Failed to open %s\n", hlslPath);
        return(1);
    }

    fseek(sourceFile, 0, SEEK_END);
    long sourceSize = ftell(sourceFile);
    fseek(sourceFile, 0, SEEK_SET);

    char* source = (char*)malloc(sourceSize);
    fread(source, 1, sourceSize, sourceFile);
    fclose(sourceFile);

    uint32 stage;
    if(strncmp(target, "vs_", 3) == 0)
    {
        stage = SHADER_STAGE_VERTEX;
    }
    else if(strncmp(target, "ps_", 3) == 0)
    {
        stage = SHADER_STAGE_PIXEL;
    }
    else
    {
        fprintf(stderr, "Unsupported shader target '%s'\n", target);
        free(source);
        return(1);
    }

    ID3DBlob* bytecode = 0;
    ID3DBlob* errors = 0;
    HRESULT result = D3DCompile(source, sourceSize, hlslPath, 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, target, D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &bytecode, &errors);

    free(source);

    if(errors)
    {
        fprintf(stderr, "%s\n", (const char*)errors->GetBufferPointer());
        errors->Release();
    }
    
    if(FAILED(result))
    {
        fprintf(stderr, "Failed to compile %s:%s\n", hlslPath, entryPoint);

        if(bytecode)
        {
            bytecode->Release();
        }

        return(1);
    }

    ShaderFileHeader header = {};
    char shaderIdentifier[4] = SHADER_IDENTIFIER;
    memcpy(header.Header.Identifier, shaderIdentifier, 4);
    header.Header.Version = SHADER_VERSION;
    header.Stage = stage;
    header.BytecodeSize = (uint32)bytecode->GetBufferSize();

    FILE* outFile = fopen(outPath, "wb");
    if(!outFile)
    {
        fprintf(stderr, "Failed to open %s for writing\n", outPath);
        bytecode->Release();
        return(1);
    }

    fwrite(&header, sizeof(header), 1, outFile);
    fwrite(bytecode->GetBufferPointer(), 1, header.BytecodeSize, outFile);

    fclose(outFile);
    printf("Cooked %s:%s (%s, %u bytes) -> %s\n", hlslPath, entryPoint, target, header.BytecodeSize, outPath);

    bytecode->Release();
    return(0);
}

int main(int argc, char** argv)
{
    if(argc < 2)
    {
        fprintf(stderr, "Usage: AgCooker.exe <font|texture|shader> ...\n");
        return(1);
    }

    const char* assetType = argv[1];

    if(strcmp(assetType, "font") == 0)
    {
        if(argc != 5)
        {
            fprintf(stderr, "Usage: AgCooker.exe font <in.fnt> <in.png> <out.aga>\n");
            return(1);
        }

        return(CookFont(argv[2], argv[3], argv[4]));
    }
    else if(strcmp(assetType, "texture") == 0)
    {
        if(argc != 4)
        {
            fprintf(stderr, "Usage: AgCooker.exe texture <in.png> <out.aga>\n");
            return(1);
        }

        return(CookTexture(argv[2], argv[3]));
    }
    else if(strcmp(assetType, "shader") == 0)
    {
        if(argc != 6)
        {
            fprintf(stderr, "Usage: AgCooker.exe shader <in.hlsl> <entry> <target> <out.aga>\n");
            return(1);
        }

        return(CookShader(argv[2], argv[3], argv[4], argv[5]));
    }

    fprintf(stderr, "Unknown asset type '%s'\n", assetType);
    return(1);
}
