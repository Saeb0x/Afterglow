#include "Engine/Renderer/Gpu.h"
#include "Engine/Renderer/Internal/GpuInternal.h"

#include <SSTL/Core/Config.h>
#include <SSTL/Core/Utility.h>
#include <SSTL/Core/Assert.h>

#include <d3d11.h>
#include <d3d11_1.h>
#include <dxgi1_6.h>

// NOTE(saeb): What a GpuPipeline points to.
struct D3D11Pipeline
{
    ID3D11VertexShader* VertexShader;
    ID3D11PixelShader* PixelShader;
    ID3D11InputLayout* InputLayout; // Null for a pipeline with no vertex attributes
    ID3D11BlendState* BlendState;
    ID3D11RasterizerState* RasterizerState;
    ID3D11DepthStencilState* DepthState;
    D3D11_PRIMITIVE_TOPOLOGY Topology;
};

struct Gpu
{
    IDXGIFactory2* Factory;
    IDXGIAdapter1* Adapter;
    ID3D11Device* Device;
    ID3D11DeviceContext* Context;
    ID3DUserDefinedAnnotation* Annotation;
    IDXGISwapChain1* SwapChain;
    ID3D11RenderTargetView* RenderTargetView;
    ID3D11DepthStencilView* DepthView;
    int32 BackBufferWidth, BackBufferHeight;
    ID3D11SamplerState* Samplers[(uint32)GpuSampler::Count];
    GpuStats Stats;
    GpuCaps Caps;
};
static Gpu GpuData;

// NOTE(saeb): Shows up in RenderDoc / PIX and in debug-layer messages, including the live-object report at shutdown.
static void D3D11SetName(ID3D11DeviceChild* object, StringView8 name)
{
    if(!object || !name.Data || name.Length == 0)
    {
        return;
    }

    object->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)name.Length, name.Data);
}

// NOTE(saeb): Every D3D11 shader is a DXBC container: "DXBC", a 16-byte checksum, a version, then its total size at byte 24. Checking the magic and size rejects truncated or garbage bytecode quietly; with the debug layer set to break on errors, passing it to D3D would stop the program instead. A flipped bit inside otherwise valid bytecode still reaches D3D's checksum.
static bool D3D11IsBytecodeValid(const uint8* bytecode, usize size)
{
    if(!bytecode || size < 32)
    {
        return(false);
    }

    if(bytecode[0] != 'D' || bytecode[1] != 'X' || bytecode[2] != 'B' || bytecode[3] != 'C')
    {
        return(false);
    }

    uint32 containerSize = (uint32)bytecode[24] | ((uint32)bytecode[25] << 8) | ((uint32)bytecode[26] << 16) | ((uint32)bytecode[27] << 24);

    return(containerSize == size);
}

static ID3D11Buffer* D3D11GpuGetBuffer(GpuBuffer buffer)
{
    return((ID3D11Buffer*)buffer.Object);
}

static ID3D11ShaderResourceView* D3D11GpuGetTexture(GpuTexture texture)
{
    return((ID3D11ShaderResourceView*)texture.Object);
}

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

// NOTE(saeb): One depth value per pixel, the same size as the back buffer. 32-bit float, because reversed depth (near 1, far 0) only gains precision with floats.
static bool D3D11CreateDepthView(int32 width, int32 height)
{
    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = (UINT)width;
    depthDesc.Height = (UINT)height;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ID3D11Texture2D* depthTexture = nullptr;
    if(FAILED(GpuData.Device->CreateTexture2D(&depthDesc, nullptr, &depthTexture)))
    {
        return(false);
    }

    if(FAILED(GpuData.Device->CreateDepthStencilView(depthTexture, nullptr, &GpuData.DepthView)))
    {
        GpuData.DepthView = nullptr;
        depthTexture->Release();

        return(false);
    }

    D3D11SetName(depthTexture, SV8(u8"DepthBuffer"));
    D3D11SetName(GpuData.DepthView, SV8(u8"DepthBufferDSV"));

    // NOTE(saeb): The view keeps the texture alive, like the back buffer's view does.
    depthTexture->Release();

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

    // NOTE(saeb): The fixed sampler set, in GpuSampler order.
    StringView8 samplerNames[] = { SV8(u8"LinearClampSampler"), SV8(u8"LinearWrapSampler"), SV8(u8"PointClampSampler"), SV8(u8"PointWrapSampler") };
    SSTL_ASSERT_STATIC_MSG(SSTL_ARRAYCOUNT(samplerNames) == (uint32)GpuSampler::Count, "Afterglow: One sampler name per GpuSampler.");
    for(uint32 index = 0; index < (uint32)GpuSampler::Count; ++index)
    {
        bool linear = (index == (uint32)GpuSampler::LinearClamp || index == (uint32)GpuSampler::LinearWrap);
        bool wrap = (index == (uint32)GpuSampler::LinearWrap || index == (uint32)GpuSampler::PointWrap);
        D3D11_TEXTURE_ADDRESS_MODE address = wrap ? D3D11_TEXTURE_ADDRESS_WRAP : D3D11_TEXTURE_ADDRESS_CLAMP;

        D3D11_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Filter = linear ? D3D11_FILTER_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_MAG_MIP_POINT;
        samplerDesc.AddressU = address;
        samplerDesc.AddressV = address;
        samplerDesc.AddressW = address;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        samplerDesc.MaxLOD = D3D11_FLOAT32_MAX; // Zeroed would lock every texture to mip 0

        if(FAILED(GpuData.Device->CreateSamplerState(&samplerDesc, &GpuData.Samplers[index])))
        {
            return(GpuInitResult::Unsupported);
        }

        D3D11SetName(GpuData.Samplers[index], samplerNames[index]);
    }

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

    if(!D3D11CreateBackBufferView() || !D3D11CreateDepthView(GpuData.BackBufferWidth, GpuData.BackBufferHeight))
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

    for(uint32 index = 0; index < (uint32)GpuSampler::Count; ++index)
    {
        if(GpuData.Samplers[index])
        {
            GpuData.Samplers[index]->Release();
            GpuData.Samplers[index] = nullptr;
        }
    }

    if(GpuData.DepthView)
    {
        GpuData.DepthView->Release();
        GpuData.DepthView = nullptr;
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

        // NOTE(saeb): The depth buffer must match the back buffer's size, so it's rebuilt with it.
        if(GpuData.DepthView)
        {
            GpuData.DepthView->Release();
            GpuData.DepthView = nullptr;
        }

        GpuData.SwapChain->ResizeBuffers(0, (UINT)width, (UINT)height, DXGI_FORMAT_UNKNOWN, GpuData.Caps.Tearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0);

        if(D3D11CreateBackBufferView() && D3D11CreateDepthView(width, height))
        {
            GpuData.BackBufferWidth = width;
            GpuData.BackBufferHeight = height;
        }
    }

    // NOTE(saeb): A failed resize leaves no view to draw into; skip the frame until device-loss handling exists.
    return(GpuData.RenderTargetView != nullptr && GpuData.DepthView != nullptr);
}

void GpuBeginPass(const GpuPassDesc* desc)
{
    // NOTE(saeb): Flip model unbinds the render target on every Present, so bind it every pass.
    GpuData.Context->OMSetRenderTargets(1, &GpuData.RenderTargetView, GpuData.DepthView);

    D3D11_VIEWPORT viewport = {};
    viewport.Width = (real32)GpuData.BackBufferWidth;
    viewport.Height = (real32)GpuData.BackBufferHeight;
    viewport.MaxDepth = 1.0f;

    GpuData.Context->RSSetViewports(1, &viewport);

    GpuBeginMarker(SV8(u8"Clear"));

    GpuData.Context->ClearRenderTargetView(GpuData.RenderTargetView, desc->ClearColor);

    // NOTE(saeb): Reversed depth: 0 is the far plane, so clearing to 0 means "nothing drawn yet, everything is nearer".
    GpuData.Context->ClearDepthStencilView(GpuData.DepthView, D3D11_CLEAR_DEPTH, 0.0f, 0);

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

bool GpuMarkersEnabled()
{
    return(GpuData.Annotation && GpuData.Annotation->GetStatus());
}

static void D3D11ReleasePipeline(D3D11Pipeline* pipeline)
{
    if(pipeline->VertexShader)
    {
        pipeline->VertexShader->Release();
    }

    if(pipeline->PixelShader)
    {
        pipeline->PixelShader->Release();
    }

    if(pipeline->InputLayout)
    {
        pipeline->InputLayout->Release();
    }

    if(pipeline->BlendState)
    {
        pipeline->BlendState->Release();
    }

    if(pipeline->RasterizerState)
    {
        pipeline->RasterizerState->Release();
    }

    if(pipeline->DepthState)
    {
        pipeline->DepthState->Release();
    }

    *pipeline = {};
}

GpuPipeline GpuCreatePipeline(StackAllocator* allocator, const GpuPipelineDesc* desc)
{
    GpuPipeline result = {};

    if(!GpuData.Device || desc->AttributeCount > GPU_MAX_VERTEX_ATTRIBUTES ||
       !D3D11IsBytecodeValid((const uint8*)desc->VertexShader, desc->VertexShaderSize) ||
       !D3D11IsBytecodeValid((const uint8*)desc->PixelShader, desc->PixelShaderSize))
    {
        return(result);
    }

    // NOTE(saeb): D3D11 matches vertex data to shader inputs by name; location N is the name ATTRIBN, so shaders declare their inputs as ATTRIB0, ATTRIB1, ...
    D3D11_INPUT_ELEMENT_DESC inputElements[GPU_MAX_VERTEX_ATTRIBUTES] = {};
    for(uint32 index = 0; index < desc->AttributeCount; ++index)
    {
        const GpuVertexAttribute* attribute = &desc->Attributes[index];

        DXGI_FORMAT format;
        switch(attribute->Format)
        {
            case GpuVertexFormat::Float:
            {
                format = DXGI_FORMAT_R32_FLOAT;
            } break;

            case GpuVertexFormat::Float2:
            {
                format = DXGI_FORMAT_R32G32_FLOAT;
            } break;

            case GpuVertexFormat::Float3:
            {
                format = DXGI_FORMAT_R32G32B32_FLOAT;
            } break;

            case GpuVertexFormat::Float4:
            {
                format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            } break;

            default:
            {
                return(result);
            }
        }

        inputElements[index].SemanticName = "ATTRIB";
        inputElements[index].SemanticIndex = attribute->Location;
        inputElements[index].Format = format;
        inputElements[index].AlignedByteOffset = attribute->Offset;
        inputElements[index].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    }

    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    if(desc->Blend == GpuBlend::Premultiplied)
    {
        // NOTE(saeb): Premultiplied "over": color = src + dst * (1 - srcAlpha); src.rgb already carries its alpha.
        blendDesc.RenderTarget[0].BlendEnable = TRUE;
        blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    }
    else if(desc->Blend == GpuBlend::Additive)
    {
        blendDesc.RenderTarget[0].BlendEnable = TRUE;
        blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    }

    D3D11_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = (desc->Cull == GpuCull::Back) ? D3D11_CULL_BACK : D3D11_CULL_NONE;
    rasterizerDesc.DepthClipEnable = TRUE; // D3D11's default is TRUE, but a zeroed desc makes it FALSE

    // NOTE(saeb): Reversed depth: nearer is larger (near plane 1, far plane 0), so a pixel passes when it's greater than what's stored.
    D3D11_DEPTH_STENCIL_DESC depthDesc = {};
    depthDesc.DepthEnable = (desc->Depth != GpuDepth::Off);
    depthDesc.DepthWriteMask = (desc->Depth == GpuDepth::TestWrite) ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
    depthDesc.DepthFunc = D3D11_COMPARISON_GREATER;

    // NOTE(saeb): Identical blend and rasterizer descs return the same shared object, so pipelines with the same states cost nothing extra.
    D3D11Pipeline pipeline = {};
    pipeline.Topology = (desc->Primitive == GpuPrimitive::Lines) ? D3D11_PRIMITIVE_TOPOLOGY_LINELIST : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    bool created = SUCCEEDED(GpuData.Device->CreateVertexShader(desc->VertexShader, desc->VertexShaderSize, nullptr, &pipeline.VertexShader)) &&
        SUCCEEDED(GpuData.Device->CreatePixelShader(desc->PixelShader, desc->PixelShaderSize, nullptr, &pipeline.PixelShader)) &&
        (desc->AttributeCount == 0 || SUCCEEDED(GpuData.Device->CreateInputLayout(inputElements, desc->AttributeCount, desc->VertexShader, desc->VertexShaderSize, &pipeline.InputLayout))) &&
        SUCCEEDED(GpuData.Device->CreateBlendState(&blendDesc, &pipeline.BlendState)) &&
        SUCCEEDED(GpuData.Device->CreateRasterizerState(&rasterizerDesc, &pipeline.RasterizerState)) &&
        SUCCEEDED(GpuData.Device->CreateDepthStencilState(&depthDesc, &pipeline.DepthState));

    D3D11Pipeline* record = created ? (D3D11Pipeline*)Allocate(allocator, Heap::Lower, sizeof(D3D11Pipeline), alignof(D3D11Pipeline)) : nullptr;
    if(!record)
    {
        D3D11ReleasePipeline(&pipeline);
        return(result);
    }

    D3D11SetName(pipeline.VertexShader, desc->DebugName);
    D3D11SetName(pipeline.PixelShader, desc->DebugName);
    D3D11SetName(pipeline.InputLayout, desc->DebugName);

    *record = pipeline;
    ++GpuData.Stats.Pipelines;
    result.Object = record;

    return(result);
}

void GpuDestroyPipeline(GpuPipeline pipeline)
{
    D3D11Pipeline* record = (D3D11Pipeline*)pipeline.Object;
    if(record)
    {
        D3D11ReleasePipeline(record);
        --GpuData.Stats.Pipelines;
    }
}

void GpuSetPipeline(GpuPipeline pipeline)
{
    D3D11Pipeline* record = (D3D11Pipeline*)pipeline.Object;
    if(!record)
    {
        return;
    }

    GpuData.Context->IASetInputLayout(record->InputLayout);
    GpuData.Context->IASetPrimitiveTopology(record->Topology);
    GpuData.Context->VSSetShader(record->VertexShader, nullptr, 0);
    GpuData.Context->PSSetShader(record->PixelShader, nullptr, 0);
    GpuData.Context->RSSetState(record->RasterizerState);
    GpuData.Context->OMSetBlendState(record->BlendState, nullptr, 0xFFFFFFFF);
    GpuData.Context->OMSetDepthStencilState(record->DepthState, 0);
}

void GpuSetVertexBuffer(GpuBuffer buffer, uint32 stride)
{
    ID3D11Buffer* object = D3D11GpuGetBuffer(buffer);
    UINT offset = 0;
    GpuData.Context->IASetVertexBuffers(0, 1, &object, &stride, &offset);
}

void GpuSetIndexBuffer(GpuBuffer buffer, GpuIndexFormat format)
{
    GpuData.Context->IASetIndexBuffer(D3D11GpuGetBuffer(buffer), (format == GpuIndexFormat::U32) ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT, 0);
}

void GpuSetConstantBuffer(uint32 slot, GpuBuffer buffer)
{
    ID3D11Buffer* object = D3D11GpuGetBuffer(buffer);
    GpuData.Context->VSSetConstantBuffers(slot, 1, &object);
    GpuData.Context->PSSetConstantBuffers(slot, 1, &object);
}

void GpuSetTexture(uint32 slot, GpuTexture texture, GpuSampler sampler)
{
    ID3D11ShaderResourceView* view = D3D11GpuGetTexture(texture);
    ID3D11SamplerState* samplerState = ((uint32)sampler < (uint32)GpuSampler::Count) ? GpuData.Samplers[(uint32)sampler] : nullptr;
    GpuData.Context->PSSetShaderResources(slot, 1, &view);
    GpuData.Context->PSSetSamplers(slot, 1, &samplerState);
}

void GpuDraw(uint32 vertexCount, uint32 firstVertex)
{
    GpuData.Context->Draw(vertexCount, firstVertex);
}

void GpuDrawIndexed(uint32 indexCount, uint32 firstIndex)
{
    GpuData.Context->DrawIndexed(indexCount, firstIndex, 0);
}
