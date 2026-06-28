#include "Win32Texture.h"
#include "Platform/Windows/Direct3D/D3D11Renderer.h"
#include "Platform/Windows/Direct3D/D3D11QuadBatcher.h"
#include "Platform/Windows/IO/Win32File.h"
#include "Assets/TextureFormat.h"

#include <string.h>

bool32 TextureLoad(D3D11RendererState* renderer, MemoryArena* transient, Texture* texture, const char* path)
{
    uint64 transientMark = transient->Used;

    ReadFileResult file = Win32ReadEntireFile(transient, path);
    if(!file.Data || file.Size < sizeof(TextureFileHeader))
    {
        transient->Used = transientMark;
        return(false);
    }

    TextureFileHeader* header = (TextureFileHeader*)file.Data;

    char textureIdentifier[4] = TEXTURE_IDENTIFIER;
    if(memcmp(header->Identifier, textureIdentifier, 4) != 0 || header->Version != TEXTURE_VERSION)
    {
        transient->Used = transientMark;
        return(false);
    }

    uint64 expectedSize = sizeof(TextureFileHeader) + ((uint64)header->Width * (uint64)header->Height * 4);
    if(file.Size != expectedSize)
    {
        transient->Used = transientMark;
        return(false);
    }

    uint8* pixels = (uint8*)file.Data + sizeof(TextureFileHeader);

    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = header->Width;
    textureDesc.Height = header->Height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA textureData = {};
    textureData.pSysMem = pixels;
    textureData.SysMemPitch = header->Width * 4;

    ID3D11Texture2D* gpuTexture = 0;
    HRESULT textureResult = renderer->Device->CreateTexture2D(&textureDesc, &textureData, &gpuTexture);

    transient->Used = transientMark;

    if(FAILED(textureResult))
    {
        return(false);
    }

    ID3D11ShaderResourceView* srv = 0;
    HRESULT viewResult = renderer->Device->CreateShaderResourceView(gpuTexture, 0, &srv);
    gpuTexture->Release();

    if(FAILED(viewResult))
    {
        return(false);
    }

    texture->Width = header->Width;
    texture->Height = header->Height;
    texture->TextureHandle = D3D11QuadBatcherRegisterTexture(srv, TEXTURED);

    return(true);
}
