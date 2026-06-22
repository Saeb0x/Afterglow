#include "D3D11Renderer.h"

static ID3D11Device* Device;
static ID3D11DeviceContext* Context;
static IDXGISwapChain1* SwapChain;
static ID3D11RenderTargetView* RenderTargetView;

static void D3D11SetViewport(int32 width, int32 height)
{
    D3D11_VIEWPORT viewport = {};
    viewport.Width = (real32)width;
    viewport.Height = (real32)height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    Context->RSSetViewports(1, &viewport);
}

static bool D3D11CreateRenderTargetView()
{
    ID3D11Texture2D* backBuffer;
    if(FAILED(SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer)))
    {
        return(false);
    }

    HRESULT result = Device->CreateRenderTargetView(backBuffer, 0, &RenderTargetView);
    backBuffer->Release();

    return(SUCCEEDED(result));
}

static IDXGIAdapter1* D3D11SelectAdapter(IDXGIFactory2* factory)
{
    // TODO(saeb): Currently always picks the adapter with the most dedicated VRAM, overriding any OS-level per-app GPU preference the user may have set. Consider respecting DXGI_GPU_PREFERENCE/EnumAdapterByGpuPreference instead of forcing highest-VRAM unconditionally.
    IDXGIAdapter1* bestAdapter = 0;
    SIZE_T bestVideoMemory = 0;

    IDXGIAdapter1* adapter;
    for(UINT index = 0; factory->EnumAdapters1(index, &adapter) != DXGI_ERROR_NOT_FOUND; ++index)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if(desc.DedicatedVideoMemory > bestVideoMemory)
        {
            if(bestAdapter)
            {
                bestAdapter->Release();
            }

            bestAdapter = adapter;
            bestVideoMemory = desc.DedicatedVideoMemory;
        }
        else
        {
            adapter->Release();
        }
    }

#if defined(AG_DEBUG)
    if(bestAdapter)
    {
        DXGI_ADAPTER_DESC1 desc;
        bestAdapter->GetDesc1(&desc);

        OutputDebugStringW(desc.Description);
        OutputDebugStringW(L"\n");
    }
#endif

    return(bestAdapter);
}

bool D3D11InitRenderer(HWND windowHandle, int32 width, int32 height)
{
    UINT factoryFlags = 0;

#if defined(AG_DEBUG)
    factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

    IDXGIFactory2* factory;
    if(FAILED(CreateDXGIFactory2(factoryFlags, __uuidof(IDXGIFactory2), (void**)&factory)))
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
                                             &Device,
                                             &supportedFeatureLevel,
                                             &Context);

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

    HRESULT swapChainResult = factory->CreateSwapChainForHwnd(Device, windowHandle, &swapChainDesc, 0, 0, &SwapChain);

    factory->Release();

    if(FAILED(swapChainResult))
    {
        return(false);
    }

    if(!D3D11CreateRenderTargetView())
    {
        return(false);
    }

    D3D11SetViewport(width, height);

    return(true);
}

void D3D11ResizeRenderer(int32 width, int32 height)
{
    Context->OMSetRenderTargets(0, 0, 0);
    RenderTargetView->Release();
    RenderTargetView = 0;

    SwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

    if(!D3D11CreateRenderTargetView())
    {
        return;
    }

    D3D11SetViewport(width, height);
}

void D3D11ShutdownRenderer()
{
#if defined(AG_DEBUG)
    ID3D11Debug* debug;
    if(SUCCEEDED(Device->QueryInterface(__uuidof(ID3D11Debug), (void**)&debug)))
    {
        debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
        debug->Release();
    }
#endif

    if(RenderTargetView)
    {
        RenderTargetView->Release();
        RenderTargetView = 0;
    }
    
    if(SwapChain)
    {
        SwapChain->Release();
        SwapChain = 0;
    }
    
    if(Context)
    {
        Context->Release();
        Context = 0;
    }
    
    if(Device)
    {
        Device->Release();
        Device = 0;
    }
}

void D3D11BeginFrame()
{
    Context->OMSetRenderTargets(1, &RenderTargetView, 0);

    real32 clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
    Context->ClearRenderTargetView(RenderTargetView, clearColor);
}

void D3D11Present()
{
    SwapChain->Present(1, 0);
}
