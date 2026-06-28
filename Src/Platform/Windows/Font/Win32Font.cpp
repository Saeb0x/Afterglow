#include "Win32Font.h"
#include "Platform/Windows/Direct3D/D3D11Renderer.h"
#include "Platform/Windows/Direct3D/D3D11QuadBatcher.h"
#include "Platform/Windows/IO/Win32File.h"

#include <string.h>

bool32 FontLoad(D3D11RendererState* renderer, MemoryArena* transient, Font* font, const char* path)
{
    uint64 transientMark = transient->Used;

    ReadFileResult file = Win32ReadEntireFile(transient, path);
    if(!file.Data || file.Size < sizeof(FontFileHeader))
    {
        transient->Used = transientMark;
        return(false);
    }

    FontFileHeader* header = (FontFileHeader*)file.Data;

    char fontIdentifier[4] = FONT_IDENTIFIER;
    if(memcmp(header->Identifier, fontIdentifier, 4) != 0 || header->Version != FONT_VERSION || header->GlyphCount != 256)
    {
        transient->Used = transientMark;
        return(false);
    }

    uint64 expectedSize = sizeof(FontFileHeader) + (sizeof(FontGlyph) * header->GlyphCount) + ((uint64)header->AtlasWidth * (uint64)header->AtlasHeight);
    if(file.Size != expectedSize)
    {
        transient->Used = transientMark;
        return(false);
    }

    font->AtlasWidth = header->AtlasWidth;
    font->AtlasHeight = header->AtlasHeight;
    font->LineHeight = header->LineHeight;

    FontGlyph* glyphs = (FontGlyph*)((uint8*)file.Data + sizeof(FontFileHeader));
    memcpy(font->Glyphs, glyphs, sizeof(FontGlyph) * 256);
    
    uint8* pixels = (uint8*)glyphs + (sizeof(FontGlyph) * header->GlyphCount);
    
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = font->AtlasWidth;
    textureDesc.Height = font->AtlasHeight;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA textureData = {};
    textureData.pSysMem = pixels;
    textureData.SysMemPitch = font->AtlasWidth; // 1 byte per pixel

    ID3D11Texture2D* texture = 0;
    HRESULT textureResult = renderer->Device->CreateTexture2D(&textureDesc, &textureData, &texture);

    transient->Used = transientMark;

    if(FAILED(textureResult))
    {
        return(false);
    }

    ID3D11ShaderResourceView* atlasSRV = 0;
    HRESULT viewResult = renderer->Device->CreateShaderResourceView(texture, 0, &atlasSRV);
    texture->Release();

    if(FAILED(viewResult))
    {
        return(false);
    }

    font->TextureHandle = D3D11QuadBatcherRegisterTexture(atlasSRV, GLYPH);
    return(true);
}
