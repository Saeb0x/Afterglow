#include "Asset.h"
#include "AssetFormat.h"
#include "Engine/Platform/File.h"

// NOTE(saeb): Reads the file into Upper heap scratch (the caller owns the frame) and checks the common header. On success, *payload points just past the header, 16-byte aligned, and *payloadSize is exact.
static AssetLoadResult AssetReadPayload(StackAllocator* allocator, StringView8 path, AssetType type, const uint8** payload, uint64* payloadSize)
{
    FileContents file;
    if(FileRead(allocator, Heap::Upper, path, &file) != FileReadResult::Ok)
    {
        return(AssetLoadResult::FileError);
    }

    if(file.Size < sizeof(AssetHeader))
    {
        return(AssetLoadResult::NotAnAsset);
    }

    const AssetHeader* header = (const AssetHeader*)file.Data;
    if(header->Magic != AG_ASSET_MAGIC)
    {
        return(AssetLoadResult::NotAnAsset);
    }

    if(header->Version != AG_ASSET_VERSION)
    {
        return(AssetLoadResult::WrongVersion);
    }

    if(header->Type != type)
    {
        return(AssetLoadResult::WrongType);
    }

    // NOTE(saeb): Exact, not "at least"; a truncated file and one with junk appended are both damaged.
    if(header->PayloadSize != file.Size - sizeof(AssetHeader))
    {
        return(AssetLoadResult::Corrupt);
    }

    *payload = (const uint8*)(header + 1);
    *payloadSize = header->PayloadSize;

    return(AssetLoadResult::Ok);
}

static AssetLoadResult AssetCreateTexture(const uint8* payload, uint64 payloadSize, StringView8 path, RendererTexture* texture)
{
    if(payloadSize < sizeof(AssetTextureHeader))
    {
        return(AssetLoadResult::Corrupt);
    }

    const AssetTextureHeader* textureHeader = (const AssetTextureHeader*)payload;
    uint32 width = textureHeader->Width;
    uint32 height = textureHeader->Height;

    if(width == 0 || height == 0 || width > AG_ASSET_MAX_TEXTURE_DIMENSION || height > AG_ASSET_MAX_TEXTURE_DIMENSION)
    {
        return(AssetLoadResult::Corrupt);
    }

    // NOTE(saeb): At most 16384 * 16384 * 4 = 1 GiB, so this can't overflow 64 bits.
    uint64 pixelBytes = (uint64)width * height * 4;
    if(payloadSize != sizeof(AssetTextureHeader) + pixelBytes)
    {
        return(AssetLoadResult::Corrupt);
    }

    RendererTexture handle = RendererCreateTexture(width, height, RendererTextureFormat::RGBA8, (const uint8*)(textureHeader + 1), path);
    if(handle == 0)
    {
        return(AssetLoadResult::RendererFailed);
    }

    *texture = handle;

    return(AssetLoadResult::Ok);
}

// NOTE(saeb): A present blob must start after the shader header, on AG_ASSET_ALIGNMENT, and end inside the payload; 64-bit math, so offset + size can't wrap.
static bool AssetShaderBlobValid(uint32 offset, uint32 size, uint64 payloadSize)
{
    return(offset >= sizeof(AssetShaderHeader) && (offset % AG_ASSET_ALIGNMENT) == 0 && (uint64)offset + size <= payloadSize);
}

// NOTE(saeb): Checks every present blob; which stages are required is up to the caller.
static AssetLoadResult AssetValidateShader(const uint8* payload, uint64 payloadSize, const AssetShaderHeader** shaderHeader)
{
    if(payloadSize < sizeof(AssetShaderHeader))
    {
        return(AssetLoadResult::Corrupt);
    }

    const AssetShaderHeader* header = (const AssetShaderHeader*)payload;

    if(header->VertexSize > 0 && !AssetShaderBlobValid(header->VertexOffset, header->VertexSize, payloadSize))
    {
        return(AssetLoadResult::Corrupt);
    }

    if(header->PixelSize > 0 && !AssetShaderBlobValid(header->PixelOffset, header->PixelSize, payloadSize))
    {
        return(AssetLoadResult::Corrupt);
    }

    *shaderHeader = header;

    return(AssetLoadResult::Ok);
}

static AssetLoadResult AssetCreateDefaultPipeline(const uint8* payload, uint64 payloadSize)
{
    const AssetShaderHeader* shaderHeader = nullptr;
    AssetLoadResult result = AssetValidateShader(payload, payloadSize, &shaderHeader);
    if(result != AssetLoadResult::Ok)
    {
        return(result);
    }

    if(shaderHeader->VertexSize == 0 || shaderHeader->PixelSize == 0)
    {
        return(AssetLoadResult::MissingStage);
    }

    if(!RendererSetDefaultPipeline(payload + shaderHeader->VertexOffset, shaderHeader->VertexSize, payload + shaderHeader->PixelOffset, shaderHeader->PixelSize))
    {
        return(AssetLoadResult::RendererFailed);
    }

    return(AssetLoadResult::Ok);
}

static AssetLoadResult AssetCreatePipeline(const uint8* payload, uint64 payloadSize, StringView8 path, RendererPipeline* pipeline)
{
    const AssetShaderHeader* shaderHeader = nullptr;
    AssetLoadResult result = AssetValidateShader(payload, payloadSize, &shaderHeader);
    if(result != AssetLoadResult::Ok)
    {
        return(result);
    }

    if(shaderHeader->PixelSize == 0)
    {
        return(AssetLoadResult::MissingStage);
    }

    RendererPipeline handle = RendererCreatePipeline(payload + shaderHeader->PixelOffset, shaderHeader->PixelSize, path);
    if(handle == 0)
    {
        return(AssetLoadResult::RendererFailed);
    }

    *pipeline = handle;

    return(AssetLoadResult::Ok);
}

static AssetLoadResult AssetCreateFont(StackAllocator* allocator, const uint8* payload, uint64 payloadSize, StringView8 path, Font* font)
{
    if(payloadSize < sizeof(AssetFontHeader))
    {
        return(AssetLoadResult::Corrupt);
    }

    const AssetFontHeader* header = (const AssetFontHeader*)payload;
    uint32 glyphCount = header->GlyphCount;
    uint32 atlasWidth = header->AtlasWidth;
    uint32 atlasHeight = header->AtlasHeight;

    if(glyphCount == 0 || glyphCount > AG_ASSET_MAX_FONT_GLYPHS || atlasWidth == 0 || atlasHeight == 0 ||
       atlasWidth > AG_ASSET_MAX_TEXTURE_DIMENSION || atlasHeight > AG_ASSET_MAX_TEXTURE_DIMENSION || !(header->PixelHeight > 0.0f))
    {
        return(AssetLoadResult::Corrupt);
    }

    // NOTE(saeb): All bounded above, so this can't overflow 64 bits.
    uint64 glyphBytes = (uint64)glyphCount * sizeof(AssetGlyph);
    uint64 atlasBytes = (uint64)atlasWidth * atlasHeight;
    if(payloadSize != sizeof(AssetFontHeader) + glyphBytes + atlasBytes)
    {
        return(AssetLoadResult::Corrupt);
    }

    const AssetGlyph* glyphs = (const AssetGlyph*)(header + 1);
    const uint8* atlas = (const uint8*)(glyphs + glyphCount);

    // NOTE(saeb): Every rectangle must lie inside the atlas, and codepoints must strictly increase, which the binary search in TextDraw relies on.
    uint32 fallbackGlyph = 0;
    for(uint32 index = 0; index < glyphCount; ++index)
    {
        const AssetGlyph* glyph = &glyphs[index];
        if((uint32)glyph->AtlasX + glyph->Width > atlasWidth || (uint32)glyph->AtlasY + glyph->Height > atlasHeight)
        {
            return(AssetLoadResult::Corrupt);
        }

        if(index > 0 && glyph->Codepoint <= glyphs[index - 1].Codepoint)
        {
            return(AssetLoadResult::Corrupt);
        }

        if(glyph->Codepoint == '?')
        {
            fallbackGlyph = index;
        }
    }

    Frame lowerFrame = GetFrame(allocator, Heap::Lower);

    TextGlyph* textGlyphs = (TextGlyph*)Allocate(allocator, Heap::Lower, glyphCount * sizeof(TextGlyph), alignof(TextGlyph));
    if(!textGlyphs)
    {
        return(AssetLoadResult::OutOfMemory);
    }

    real32 inverseWidth = 1.0f / (real32)atlasWidth;
    real32 inverseHeight = 1.0f / (real32)atlasHeight;
    for(uint32 index = 0; index < glyphCount; ++index)
    {
        const AssetGlyph* source = &glyphs[index];
        TextGlyph* glyph = &textGlyphs[index];
        glyph->Codepoint = source->Codepoint;
        glyph->U0 = (real32)source->AtlasX * inverseWidth;
        glyph->V0 = (real32)source->AtlasY * inverseHeight;
        glyph->U1 = (real32)(source->AtlasX + source->Width) * inverseWidth;
        glyph->V1 = (real32)(source->AtlasY + source->Height) * inverseHeight;
        glyph->OffsetX = source->OffsetX;
        glyph->OffsetY = source->OffsetY;
        glyph->Width = (real32)source->Width;
        glyph->Height = (real32)source->Height;
        glyph->Advance = source->Advance;
    }

    RendererTexture atlasTexture = RendererCreateTexture(atlasWidth, atlasHeight, RendererTextureFormat::R8, atlas, path);
    if(atlasTexture == 0)
    {
        // NOTE(saeb): Give the glyph table back, so a failed load leaves the allocator exactly as it was.
        ReleaseFrame(allocator, lowerFrame);
        return(AssetLoadResult::RendererFailed);
    }

    font->Atlas = atlasTexture;
    font->Glyphs = textGlyphs;
    font->GlyphCount = glyphCount;
    font->FallbackGlyph = fallbackGlyph;
    font->PixelHeight = header->PixelHeight;
    font->Ascent = header->Ascent;
    font->Descent = header->Descent;
    font->LineHeight = header->LineHeight;

    return(AssetLoadResult::Ok);
}

AssetLoadResult AssetLoadTexture(StackAllocator* allocator, StringView8 path, RendererTexture* texture)
{
    *texture = 0;

    // NOTE(saeb): The GPU copies the pixels at creation, so the whole file is scratch.
    Frame scratch = GetFrame(allocator, Heap::Upper);

    const uint8* payload = nullptr;
    uint64 payloadSize = 0;
    AssetLoadResult result = AssetReadPayload(allocator, path, AssetType::Texture, &payload, &payloadSize);
    if(result == AssetLoadResult::Ok)
    {
        result = AssetCreateTexture(payload, payloadSize, path, texture);
    }

    ReleaseFrame(allocator, scratch);

    return(result);
}

AssetLoadResult AssetLoadDefaultPipeline(StackAllocator* allocator, StringView8 path)
{
    // NOTE(saeb): The driver keeps its own copy of the bytecode, so the whole file is scratch.
    Frame scratch = GetFrame(allocator, Heap::Upper);

    const uint8* payload = nullptr;
    uint64 payloadSize = 0;
    AssetLoadResult result = AssetReadPayload(allocator, path, AssetType::Shader, &payload, &payloadSize);
    if(result == AssetLoadResult::Ok)
    {
        result = AssetCreateDefaultPipeline(payload, payloadSize);
    }

    ReleaseFrame(allocator, scratch);

    return(result);
}

AssetLoadResult AssetLoadPipeline(StackAllocator* allocator, StringView8 path, RendererPipeline* pipeline)
{
    *pipeline = 0;

    // NOTE(saeb): The driver keeps its own copy of the bytecode, so the whole file is scratch.
    Frame scratch = GetFrame(allocator, Heap::Upper);

    const uint8* payload = nullptr;
    uint64 payloadSize = 0;
    AssetLoadResult result = AssetReadPayload(allocator, path, AssetType::Shader, &payload, &payloadSize);
    if(result == AssetLoadResult::Ok)
    {
        result = AssetCreatePipeline(payload, payloadSize, path, pipeline);
    }

    ReleaseFrame(allocator, scratch);

    return(result);
}

AssetLoadResult AssetLoadFont(StackAllocator* allocator, StringView8 path, Font* font)
{
    *font = {};

    // NOTE(saeb): Like textures, the file is scratch: the GPU copies the atlas, and only the glyph table is kept, in the Lower heap.
    Frame scratch = GetFrame(allocator, Heap::Upper);

    const uint8* payload = nullptr;
    uint64 payloadSize = 0;
    AssetLoadResult result = AssetReadPayload(allocator, path, AssetType::Font, &payload, &payloadSize);
    if(result == AssetLoadResult::Ok)
    {
        result = AssetCreateFont(allocator, payload, payloadSize, path, font);
    }

    ReleaseFrame(allocator, scratch);

    return(result);
}

StringView8 AssetDescribeResult(AssetLoadResult result)
{
    switch(result)
    {
        case AssetLoadResult::Ok:
        {
            return(SV8(u8"ok"));
        }

        case AssetLoadResult::FileError:
        {
            return(SV8(u8"the file couldn't be read (missing, inaccessible, or out of memory)"));
        }

        case AssetLoadResult::NotAnAsset:
        {
            return(SV8(u8"the file isn't a cooked Afterglow asset"));
        }

        case AssetLoadResult::WrongVersion:
        {
            return(SV8(u8"the file was cooked for a different engine version; recook the assets"));
        }

        case AssetLoadResult::WrongType:
        {
            return(SV8(u8"the file is the wrong kind of asset"));
        }

        case AssetLoadResult::MissingStage:
        {
            return(SV8(u8"the shader is missing an entry point (VSMain or PSMain)"));
        }

        case AssetLoadResult::Corrupt:
        {
            return(SV8(u8"the file is truncated or damaged"));
        }

        case AssetLoadResult::RendererFailed:
        {
            return(SV8(u8"the renderer couldn't create it"));
        }

        case AssetLoadResult::OutOfMemory:
        {
            return(SV8(u8"there wasn't enough memory for it"));
        }
    }

    return(SV8(u8"unknown error"));
}
