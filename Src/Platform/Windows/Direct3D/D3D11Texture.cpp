#include "D3D11Texture.h"

#include <wincodec.h>

static IWICImagingFactory* WICFactory;

bool32 D3D11TextureInit()
{
    HRESULT result = CoCreateInstance(CLSID_WICImagingFactory, 0, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&WICFactory));
    return(SUCCEEDED(result));
}

bool32 D3D11LoadTexture(MemoryArena* transientArena, const wchar_t* path, ID3D11ShaderResourceView** outSRV, int32* outWidth, int32* outHeight)
{
    // NOTE(saeb): WIC picks the codec by content.
    IWICBitmapDecoder* decoder = 0;
    if(FAILED(WICFactory->CreateDecoderFromFilename(path, 0, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder)))
    {
        return(false);
    }

    IWICBitmapFrameDecode* frame = 0;
    if(FAILED(decoder->GetFrame(0, &frame)))
    {
        decoder->Release();
        return(false);
    }

    IWICFormatConverter* converter = 0;
    if(FAILED(WICFactory->CreateFormatConverter(&converter)))
    {
        frame->Release();
        decoder->Release();
        return(false);
    }

    // NOTE(saeb): Force the source (whatever it was) to canonical 32-bit RGBA.
    if(FAILED(converter->Initialize(frame, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, 0, 0.0, WICBitmapPaletteTypeCustom)))
    {
        converter->Release();
        frame->Release();
        decoder->Release();
        return(false);
    }

    // NOTE(saeb): Copy decoded pixels into a transient buffer.
    UINT width, height;
    converter->GetSize(&width, &height);

    UINT pitch = width * 4; // RGBA8 = 4 bytes/pixel
    UINT imageSize = pitch * height;

    uint64 transientMark = transientArena->Used;
    uint8* pixels = (uint8*)PushSize(transientArena, imageSize);

    HRESULT copyResult = converter->CopyPixels(0, pitch, imageSize, pixels);

    converter->Release();
    frame->Release();
    decoder->Release();

    if(FAILED(copyResult))
    {
        transientArena->Used = transientMark;
        return(false);
    }

    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA textureData = {};
    textureData.pSysMem = pixels;
    textureData.SysMemPitch = pitch;

    ID3D11Texture2D* texture = 0;
    HRESULT textureResult = Device->CreateTexture2D(&textureDesc, &textureData, &texture);

    // NOTE(saeb): Pixels are copied (or the call failed) - transient is dead either way.
    transientArena->Used = transientMark;

    if(FAILED(textureResult))
    {
        return(false);
    }

    HRESULT viewResult = Device->CreateShaderResourceView(texture, 0, outSRV);
    texture->Release();

    if(FAILED(viewResult))
    {
        return(false);
    }

    if(outWidth)
    {
        *outWidth = (int32)width;
    }

    if(outHeight)
    {
        *outHeight = (int32)height;
    }

    return(true);
}
