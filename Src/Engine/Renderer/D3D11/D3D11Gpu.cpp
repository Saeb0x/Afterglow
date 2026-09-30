#include "Engine/Renderer/D3D11/D3D11Gpu.h"
#include "Engine/Renderer/Gpu.h"
#include "Engine/Renderer/Internal/GpuInternal.h"

#include <SSTL/Core/Config.h>
#include <SSTL/Core/Utility.h>

#include <dxgi1_6.h>

struct Gpu
{
    IDXGIFactory2* Factory;
    IDXGIAdapter1* Adapter;
    ID3D11Device* Device;
    ID3D11DeviceContext* Context;
    ID3DUserDefinedAnnotation* Annotation;
    IDXGISwapChain1* SwapChain;
    ID3D11RenderTargetView* RenderTargetView;
    int32 BackBufferWidth, BackBufferHeight;
    GpuStats Stats;
    GpuCaps Caps;
};
static Gpu GpuData;

static bool D3D11CreateBackBufferView()
{
    ID3D11Texture2D* backBuffer = nullptr;
    if(FAILED(GpuData.SwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))))
    {
        return(false);
    }

    if(FAILED(GpuData.Device->CreateRenderTargetView(backBuffer, nullptr, &GpuData.RenderTargetView)))
    {
        GpuData.RenderTargetView = nullptr;
        backBuffer->Release();

        return(false);
    }

    D3D11SetName(backBuffer, SV8(u8"BackBuffer"));
    D3D11SetName(GpuData.RenderTargetView, SV8(u8"BackBufferRTV"));

    backBuffer->Release();

    return(true);
}

GpuInitResult GpuInit(const GpuDesc* desc)
{
    // NOTE(saeb): Factory first, so we choose the adapter; the swap chain later comes from this same factory, which is the adapter's parent (a mismatched factory fails).
    UINT factoryFlags = 0;
#if SSTL_DEBUG
    factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

    if(FAILED(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&GpuData.Factory))))
    {
        // NOTE(saeb): The debug factory needs the optional "Graphics Tools" Windows feature; fall back to a normal one.
        if(FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(&GpuData.Factory))))
        {
            return(GpuInitResult::NoGpu);
        }
    }

    // NOTE(saeb): IDXGIFactory6 (minimum Windows 10, version 1803) can order adapters so the high-performance GPU comes first; older systems fall back to plain enumeration order.
    IDXGIFactory6* factory6 = nullptr;
    GpuData.Factory->QueryInterface(IID_PPV_ARGS(&factory6));

    for(UINT adapterIndex = 0; ; ++adapterIndex)
    {
        HRESULT result;
        if(factory6)
        {
            result = factory6->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&GpuData.Adapter));
        }
        else
        {
            result = GpuData.Factory->EnumAdapters1(adapterIndex, &GpuData.Adapter);
        }

        if(FAILED(result))
        {
            GpuData.Adapter = nullptr;

            break;
        }

        // NOTE(saeb): Skip the Microsoft Basic Render Driver (CPU rasterizer); it's always enumerated but never what we want.
        DXGI_ADAPTER_DESC1 adapterDesc;
        GpuData.Adapter->GetDesc1(&adapterDesc);

        if(!(adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE))
        {
            OutputDebugStringW(L"[Afterglow] GPU: ");
            OutputDebugStringW(adapterDesc.Description);
            OutputDebugStringW(L"\n");

            GpuData.Caps.VideoMemory = (uint64)adapterDesc.DedicatedVideoMemory;
            if(WideCharToMultiByte(CP_UTF8, 0, adapterDesc.Description, -1, (LPSTR)GpuData.Caps.Name, sizeof(GpuData.Caps.Name), nullptr, nullptr) == 0)
            {
                GpuData.Caps.Name[0] = 0;
            }

            break;
        }

        GpuData.Adapter->Release();
        GpuData.Adapter = nullptr;
    }

    if(factory6)
    {
        factory6->Release();
    }

    if(!GpuData.Adapter)
    {
        return(GpuInitResult::NoGpu);
    }

    UINT deviceFlags = 0;
#if SSTL_DEBUG
    deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    // NOTE(saeb): "Unsupported" on this backend means no feature level 11.0.
    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };

    // NOTE(saeb): Driver type must be UNKNOWN when passing an explicit adapter; HARDWARE is only for a null adapter.
    HRESULT result = D3D11CreateDevice(GpuData.Adapter,
                                       D3D_DRIVER_TYPE_UNKNOWN,
                                       nullptr,
                                       deviceFlags,
                                       featureLevels,
                                       SSTL_ARRAYCOUNT(featureLevels),
                                       D3D11_SDK_VERSION,
                                       &GpuData.Device,
                                       nullptr,
                                       &GpuData.Context);
#if SSTL_DEBUG
    // NOTE(saeb): The debug layer ships with the optional "Graphics Tools" Windows feature; run without it rather than fail.
    if(result == DXGI_ERROR_SDK_COMPONENT_MISSING)
    {
        OutputDebugStringW(L"[Afterglow] D3D11 debug layer not installed (Settings > Optional features > Graphics Tools).\n");
        deviceFlags &= ~D3D11_CREATE_DEVICE_DEBUG;
        result = D3D11CreateDevice(GpuData.Adapter,
                                   D3D_DRIVER_TYPE_UNKNOWN,
                                   nullptr,
                                   deviceFlags,
                                   featureLevels,
                                   SSTL_ARRAYCOUNT(featureLevels),
                                   D3D11_SDK_VERSION,
                                   &GpuData.Device,
                                   nullptr,
                                   &GpuData.Context);
    }
#endif

    if(FAILED(result))
    {
        return(GpuInitResult::Unsupported);
    }

    // NOTE(saeb): Optional; groups calls in RenderDoc / PIX. A failure just means no markers.
    GpuData.Context->QueryInterface(IID_PPV_ARGS(&GpuData.Annotation));

#if SSTL_DEBUG
    // NOTE(saeb): Stop in the debugger on the exact API call that misuses D3D, instead of finding out from a black screen.
    ID3D11InfoQueue* infoQueue = nullptr;
    if(SUCCEEDED(GpuData.Device->QueryInterface(IID_PPV_ARGS(&infoQueue))))
    {
        if(IsDebuggerPresent())
        {
            infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
            infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
        }

        infoQueue->Release();
    }
#endif

    GpuData.Caps.MaxTextureSize = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;

    // NOTE(saeb): Tearing is what lets VSync-off actually present immediately on flip-model swap chains (needs Windows 10 + driver support).
    IDXGIFactory5* factory5 = nullptr;
    if(SUCCEEDED(GpuData.Factory->QueryInterface(IID_PPV_ARGS(&factory5))))
    {
        BOOL allowTearing = FALSE;

        if(SUCCEEDED(factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing))))
        {
            GpuData.Caps.Tearing = (allowTearing == TRUE);
        }

        factory5->Release();
    }

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = 0;
    swapChainDesc.Height = 0;
    swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapChainDesc.SampleDesc.Count = 1; // Flip model can't be multisampled; MSAA would be a separate target resolved into this one
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 2; // Flip model minimum
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
    swapChainDesc.Flags = GpuData.Caps.Tearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

    HWND windowHandle = (HWND)desc->WindowHandle;
    if(FAILED(GpuData.Factory->CreateSwapChainForHwnd(GpuData.Device, windowHandle, &swapChainDesc, nullptr, nullptr, &GpuData.SwapChain)))
    {
        return(GpuInitResult::SwapChainFailed);
    }

    // NOTE(saeb): Fullscreen is the window layer's job (WindowFlags_Fullscreen); stop DXGI from hijacking Alt+Enter.
    GpuData.Factory->MakeWindowAssociation(windowHandle, DXGI_MWA_NO_ALT_ENTER);

    GpuData.SwapChain->GetDesc1(&swapChainDesc);
    GpuData.BackBufferWidth = (int32)swapChainDesc.Width;
    GpuData.BackBufferHeight = (int32)swapChainDesc.Height;

    if(!D3D11CreateBackBufferView())
    {
        return(GpuInitResult::SwapChainFailed);
    }

    return(GpuInitResult::Ok);
}

void GpuShutdown()
{
    if(GpuData.Context)
    {
        GpuData.Context->ClearState();
        GpuData.Context->Flush();
    }

    if(GpuData.RenderTargetView)
    {
        GpuData.RenderTargetView->Release();
        GpuData.RenderTargetView = nullptr;
    }

    if(GpuData.SwapChain)
    {
        GpuData.SwapChain->Release();
        GpuData.SwapChain = nullptr;
    }

    if(GpuData.Annotation)
    {
        GpuData.Annotation->Release();
        GpuData.Annotation = nullptr;
    }

    if(GpuData.Context)
    {
        GpuData.Context->Release();
        GpuData.Context = nullptr;
    }

#if SSTL_DEBUG
    // NOTE(saeb): Anything listed here besides the device itself is a leaked COM reference.
    ID3D11Debug* debug = nullptr;
    if(GpuData.Device && SUCCEEDED(GpuData.Device->QueryInterface(IID_PPV_ARGS(&debug))))
    {
        debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL | D3D11_RLDO_IGNORE_INTERNAL);
        debug->Release();
    }
#endif

    if(GpuData.Device)
    {
        GpuData.Device->Release();
        GpuData.Device = nullptr;
    }

    if(GpuData.Adapter)
    {
        GpuData.Adapter->Release();
        GpuData.Adapter = nullptr;
    }

    if(GpuData.Factory)
    {
        GpuData.Factory->Release();
        GpuData.Factory = nullptr;
    }
}

void GpuGetCaps(GpuCaps* caps)
{
    *caps = GpuData.Caps;
}

GpuBuffer GpuCreateBuffer(const GpuBufferDesc* desc)
{
    GpuBuffer buffer = {};

    bool immutable = (desc->Usage == GpuUsage::Immutable);
    if(!GpuData.Device || desc->Size == 0 || (immutable && !desc->Data))
    {
        return(buffer);
    }

    UINT bindFlags;
    switch(desc->Type)
    {
        case GpuBufferType::Vertex:
        {
            bindFlags = D3D11_BIND_VERTEX_BUFFER;
        } break;

        case GpuBufferType::Index:
        {
            bindFlags = D3D11_BIND_INDEX_BUFFER;
        } break;

        case GpuBufferType::Constant:
        {
            if(desc->Size % 16 != 0)
            {
                return(buffer);
            }

            bindFlags = D3D11_BIND_CONSTANT_BUFFER;
        } break;

        default:
        {
            return(buffer);
        }
    }

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = desc->Size;
    bufferDesc.Usage = immutable ? D3D11_USAGE_IMMUTABLE : D3D11_USAGE_DYNAMIC;
    bufferDesc.BindFlags = bindFlags;
    bufferDesc.CPUAccessFlags = immutable ? 0 : D3D11_CPU_ACCESS_WRITE;

    D3D11_SUBRESOURCE_DATA data = {};
    data.pSysMem = desc->Data;

    ID3D11Buffer* object = nullptr;
    if(FAILED(GpuData.Device->CreateBuffer(&bufferDesc, desc->Data ? &data : nullptr, &object)))
    {
        return(buffer);
    }

    D3D11SetName(object, desc->DebugName);

    ++GpuData.Stats.Buffers;
    buffer.Object = object;

    return(buffer);
}

void GpuDestroyBuffer(GpuBuffer buffer)
{
    ID3D11Buffer* object = D3D11GpuGetBuffer(buffer);
    if(object)
    {
        object->Release();
        --GpuData.Stats.Buffers;
    }
}

void* GpuMapBuffer(GpuBuffer buffer)
{
    ID3D11Buffer* object = D3D11GpuGetBuffer(buffer);
    if(!object)
    {
        return(nullptr);
    }

    D3D11_MAPPED_SUBRESOURCE mapped;
    if(FAILED(GpuData.Context->Map(object, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
    {
        return(nullptr);
    }

    return(mapped.pData);
}

void GpuUnmapBuffer(GpuBuffer buffer)
{
    ID3D11Buffer* object = D3D11GpuGetBuffer(buffer);
    if(object)
    {
        GpuData.Context->Unmap(object, 0);
    }
}

GpuTexture GpuCreateTexture(const GpuTextureDesc* desc)
{
    GpuTexture texture = {};

    DXGI_FORMAT textureFormat;
    uint32 bytesPerPixel;
    switch(desc->Format)
    {
        case GpuFormat::RGBA8:
        {
            textureFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
            bytesPerPixel = 4;
        } break;

        case GpuFormat::R8:
        {
            textureFormat = DXGI_FORMAT_R8_UNORM;
            bytesPerPixel = 1;
        } break;

        default:
        {
            return(texture);
        }
    }

    if(!GpuData.Device || !desc->Data || desc->Width == 0 || desc->Height == 0 || desc->Width > GpuData.Caps.MaxTextureSize || desc->Height > GpuData.Caps.MaxTextureSize)
    {
        return(texture);
    }

    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = desc->Width;
    textureDesc.Height = desc->Height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = textureFormat;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA textureData = {};
    textureData.pSysMem = desc->Data;
    textureData.SysMemPitch = desc->Width * bytesPerPixel;

    ID3D11Texture2D* object = nullptr;
    if(FAILED(GpuData.Device->CreateTexture2D(&textureDesc, &textureData, &object)))
    {
        return(texture);
    }

    ID3D11ShaderResourceView* view = nullptr;
    HRESULT viewResult = GpuData.Device->CreateShaderResourceView(object, nullptr, &view);

    D3D11SetName(object, desc->DebugName);
    object->Release(); // The view holds its own reference to the texture

    if(FAILED(viewResult))
    {
        return(texture);
    }

    D3D11SetName(view, desc->DebugName);

    ++GpuData.Stats.Textures;
    texture.Object = view;

    return(texture);
}

void GpuDestroyTexture(GpuTexture texture)
{
    ID3D11ShaderResourceView* view = D3D11GpuGetTexture(texture);
    if(view)
    {
        view->Release();
        --GpuData.Stats.Textures;
    }
}

void GpuGetStats(GpuStats* stats)
{
    *stats = GpuData.Stats;
}

bool GpuBeginFrame(int32 width, int32 height)
{
    if((width != GpuData.BackBufferWidth || height != GpuData.BackBufferHeight) && width > 0 && height > 0)
    {
        // NOTE(saeb): ResizeBuffers fails while anything still references the old back buffer, so unbind and release the view first.
        GpuData.Context->OMSetRenderTargets(0, nullptr, nullptr);
        if(GpuData.RenderTargetView)
        {
            GpuData.RenderTargetView->Release();
            GpuData.RenderTargetView = nullptr;
        }

        GpuData.SwapChain->ResizeBuffers(0, (UINT)width, (UINT)height, DXGI_FORMAT_UNKNOWN, GpuData.Caps.Tearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0);

        if(D3D11CreateBackBufferView())
        {
            GpuData.BackBufferWidth = width;
            GpuData.BackBufferHeight = height;
        }
    }

    // NOTE(saeb): A failed resize leaves no view to draw into; skip the frame until device-loss handling exists.
    return(GpuData.RenderTargetView != nullptr);
}

void GpuBeginPass(const GpuPassDesc* desc)
{
    // NOTE(saeb): Flip model unbinds the render target on every Present, so bind it every pass.
    GpuData.Context->OMSetRenderTargets(1, &GpuData.RenderTargetView, nullptr);

    D3D11_VIEWPORT viewport = {};
    viewport.Width = (real32)GpuData.BackBufferWidth;
    viewport.Height = (real32)GpuData.BackBufferHeight;
    viewport.MaxDepth = 1.0f;

    GpuData.Context->RSSetViewports(1, &viewport);

    GpuBeginMarker(SV8(u8"Clear"));
    GpuData.Context->ClearRenderTargetView(GpuData.RenderTargetView, desc->ClearColor);
    GpuEndMarker();
}

void GpuEndPass()
{
}

void GpuPresent(bool vsync)
{
    if(vsync)
    {
        GpuData.SwapChain->Present(1, 0);
    }
    else
    {
        GpuData.SwapChain->Present(0, GpuData.Caps.Tearing ? DXGI_PRESENT_ALLOW_TEARING : 0);
    }
}

void GpuGetBackBufferSize(int32* width, int32* height)
{
    *width = GpuData.BackBufferWidth;
    *height = GpuData.BackBufferHeight;
}

void GpuBeginMarker(StringView8 name)
{
    if(!GpuData.Annotation)
    {
        return;
    }

    wchar_t wide[128];
    int length = MultiByteToWideChar(CP_UTF8, 0, (LPCCH)name.Data, (int)name.Length, wide, SSTL_ARRAYCOUNT(wide) - 1);
    wide[length] = 0;
    GpuData.Annotation->BeginEvent(wide);
}

void GpuEndMarker()
{
    if(GpuData.Annotation)
    {
        GpuData.Annotation->EndEvent();
    }
}

ID3D11Device* D3D11GpuGetDevice()
{
    return(GpuData.Device);
}

ID3D11DeviceContext* D3D11GpuGetContext()
{
    return(GpuData.Context);
}

ID3DUserDefinedAnnotation* D3D11GpuGetAnnotation()
{
    return(GpuData.Annotation);
}

ID3D11Buffer* D3D11GpuGetBuffer(GpuBuffer buffer)
{
    return((ID3D11Buffer*)buffer.Object);
}

ID3D11ShaderResourceView* D3D11GpuGetTexture(GpuTexture texture)
{
    return((ID3D11ShaderResourceView*)texture.Object);
}

// NOTE(saeb): Shows up in RenderDoc / PIX and in debug-layer messages, including the live-object report at shutdown.
void D3D11SetName(ID3D11DeviceChild* object, StringView8 name)
{
    if(!object || !name.Data || name.Length == 0)
    {
        return;
    }

    object->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)name.Length, name.Data);
}
