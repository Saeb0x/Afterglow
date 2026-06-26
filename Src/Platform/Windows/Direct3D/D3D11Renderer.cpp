#include "D3D11Renderer.h"

static void D3D11SetViewport(D3D11RendererState* renderer, int32 width, int32 height)
{
    D3D11_VIEWPORT viewport = {};
    viewport.Width = (real32)width;
    viewport.Height = (real32)height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    renderer->Context->RSSetViewports(1, &viewport);
}

static bool32 D3D11CreateRenderTargetView(D3D11RendererState* renderer)
{
    ID3D11Texture2D* backBuffer;
    if(FAILED(renderer->SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer)))
    {
        return(false);
    }
    
    HRESULT result = renderer->Device->CreateRenderTargetView(backBuffer, 0, &renderer->RenderTargetView);
    backBuffer->Release();

    return(SUCCEEDED(result));
}

static bool32 D3D11CreateDepthStencilView(D3D11RendererState* renderer, int32 width, int32 height)
{
    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    if(FAILED(renderer->Device->CreateTexture2D(&depthDesc, 0, &renderer->DepthStencilBuffer)))
    {
        return(false);
    }

    HRESULT result = renderer->Device->CreateDepthStencilView(renderer->DepthStencilBuffer, 0, &renderer->DepthStencilView);

    return(SUCCEEDED(result));
}

static IDXGIAdapter1* D3D11SelectAdapter(IDXGIFactory6* factory)
{
    // NOTE(saeb): Ask DXGI for adapters in preference order. HIGH_PERFORMANCE respects the user's per-app GPU choice and otherwise favors the discrete GPU, so we just take the first hardware adapter it returns.
    IDXGIAdapter1* adapter;

    for(UINT index = 0; factory->EnumAdapterByGpuPreference(index, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, __uuidof(IDXGIAdapter1), (void**)&adapter) != DXGI_ERROR_NOT_FOUND; ++index)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if(!(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE))
        {
            return(adapter);
        }

        adapter->Release();
    }

    return(0);
}

bool32 D3D11InitRenderer(D3D11RendererState* renderer, HWND windowHandle, int32 width, int32 height)
{
    UINT factoryFlags = 0;

#if defined(AG_DEBUG)
    factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

    IDXGIFactory6* factory;
    if(FAILED(CreateDXGIFactory2(factoryFlags, __uuidof(IDXGIFactory6), (void**)&factory)))
    {
        return(false);
    }

    IDXGIAdapter1* adapter = D3D11SelectAdapter(factory);
    if(!adapter)
    {
        factory->Release();
        return(false);
    }

    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
    D3D_FEATURE_LEVEL supportedFeatureLevel;

    UINT deviceFlags = 0;

#if defined(AG_DEBUG)
    deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    HRESULT deviceResult = D3D11CreateDevice(adapter,
                                             D3D_DRIVER_TYPE_UNKNOWN,
                                             0,
                                             deviceFlags,
                                             featureLevels,
                                             ArrayCount(featureLevels),
                                             D3D11_SDK_VERSION,
                                             &renderer->Device,
                                             &supportedFeatureLevel,
                                             &renderer->Context);

    adapter->Release();

    if(FAILED(deviceResult))
    {
        factory->Release();
        return(false);
    }

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

    HRESULT swapChainResult = factory->CreateSwapChainForHwnd(renderer->Device, windowHandle, &swapChainDesc, 0, 0, &renderer->SwapChain);

    factory->Release();

    if(FAILED(swapChainResult))
    {
        return(false);
    }

    if(!D3D11CreateRenderTargetView(renderer))
    {
        return(false);
    }

    if(!D3D11CreateDepthStencilView(renderer, width, height))
    {
        return(false);
    }

    D3D11SetViewport(renderer, width, height);

    return(true);
}

void D3D11ResizeRenderer(D3D11RendererState* renderer, int32 width, int32 height)
{
    if(width <= 0 || height <= 0)
    {
        return;
    }

    renderer->Context->OMSetRenderTargets(0, 0, 0);

    renderer->RenderTargetView->Release();
    renderer->RenderTargetView = 0;

    renderer->DepthStencilView->Release();
    renderer->DepthStencilView = 0;
    renderer->DepthStencilBuffer->Release();
    renderer->DepthStencilBuffer = 0;

    renderer->SwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

    if(!D3D11CreateRenderTargetView(renderer))
    {
        return;
    }

    if(!D3D11CreateDepthStencilView(renderer, width, height))
    {
        return;
    }

    D3D11SetViewport(renderer, width, height);
}

void D3D11ShutdownRenderer(D3D11RendererState* renderer)
{
#if defined(AG_DEBUG)
    if(renderer->Device)
    {
        ID3D11Debug* debug;
        if(SUCCEEDED(renderer->Device->QueryInterface(__uuidof(ID3D11Debug), (void**)&debug)))
        {
            debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
            debug->Release();
        }
    }    
#endif

    if(renderer->DepthStencilView)
    {
        renderer->DepthStencilView->Release();
        renderer->DepthStencilView = 0;
    }

    if(renderer->DepthStencilBuffer)
    {
        renderer->DepthStencilBuffer->Release();
        renderer->DepthStencilBuffer = 0;
    }

    if(renderer->RenderTargetView)
    {
        renderer->RenderTargetView->Release();
        renderer->RenderTargetView = 0;
    }

    if(renderer->SwapChain)
    {
        renderer->SwapChain->Release();
        renderer->SwapChain = 0;
    }
    
    if(renderer->Context)
    {
        renderer->Context->Release();
        renderer->Context = 0;
    }
    
    if(renderer->Device)
    {
        renderer->Device->Release();
        renderer->Device = 0;
    }
}

void D3D11BeginFrame(D3D11RendererState* renderer)
{
    renderer->Context->OMSetRenderTargets(1, &renderer->RenderTargetView, renderer->DepthStencilView);

    real32 clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    renderer->Context->ClearRenderTargetView(renderer->RenderTargetView, clearColor);
    renderer->Context->ClearDepthStencilView(renderer->DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void D3D11Present(D3D11RendererState* renderer)
{
    renderer->SwapChain->Present(1, 0);
}
