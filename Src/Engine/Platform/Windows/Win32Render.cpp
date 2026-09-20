#include "Win32Render.h"

#include <SSTL/Core/Config.h>
#include <SSTL/Core/Utility.h>

#include <d3d11.h>
#include <d3dcompiler.h>

struct Vertex
{
    real32 Position[2];
    real32 Color[4];
};

struct Renderer
{
    ID3D11Device* Device;
    ID3D11DeviceContext* Context;
    IDXGISwapChain* SwapChain;
    ID3D11RenderTargetView* RenderTargetView;
    ID3D11VertexShader* VertexShader;
    ID3D11PixelShader* PixelShader;
    ID3D11InputLayout* InputLayout;
    ID3D11Buffer* VertexBuffer;
    ID3D11RasterizerState* RasterizerState;
    uint32 Width, Height;
};
static Renderer RendererData = {};

static const char QuadShaderSource[] = R"HLSL(
struct VSInput
{
    float2 Position : POSITION;
    float4 Color : COLOR;
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
};

PSInput VSMain(VSInput input)
{
    PSInput output;
    output.Position = float4(input.Position, 0.0f, 1.0f);
    output.Color = input.Color;
    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return input.Color;
}
)HLSL";

bool Win32RenderInit(HWND windowHandle, uint32 width, uint32 height)
{
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferDesc.Width = width;
    swapChainDesc.BufferDesc.Height = height;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.OutputWindow = windowHandle;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    UINT deviceFlags = 0;
#if SSTL_DEBUG
    deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
    D3D_FEATURE_LEVEL featureLevel;

    HRESULT result = D3D11CreateDeviceAndSwapChain(nullptr,
                                                   D3D_DRIVER_TYPE_HARDWARE,
                                                   nullptr,
                                                   deviceFlags,
                                                   featureLevels,
                                                   SSTL_ARRAYCOUNT(featureLevels),
                                                   D3D11_SDK_VERSION,
                                                   &swapChainDesc,
                                                   &RendererData.SwapChain,
                                                   &RendererData.Device,
                                                   &featureLevel,
                                                   &RendererData.Context);

    if(!SUCCEEDED(result))
    {
        return(false);
    }

    ID3D11Texture2D* backBuffer;
    RendererData.SwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));

    RendererData.Device->CreateRenderTargetView(backBuffer, nullptr, &RendererData.RenderTargetView);
    backBuffer->Release();

    RendererData.Context->OMSetRenderTargets(1, &RendererData.RenderTargetView, nullptr);

    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = (real32)width;
    viewport.Height = (real32)height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    RendererData.Context->RSSetViewports(1, &viewport);

    RendererData.Width = width;
    RendererData.Height = height;

    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;

    HRESULT vsResult = D3DCompile(QuadShaderSource, sizeof(QuadShaderSource) - 1, nullptr, nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vsBlob, &errorBlob);
    if(!SUCCEEDED(vsResult))
    {
        if(errorBlob)
        {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
            errorBlob->Release();
        }

        return(false);
    }

    HRESULT psResult = D3DCompile(QuadShaderSource, sizeof(QuadShaderSource) - 1, nullptr, nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &psBlob, &errorBlob);
    if(!SUCCEEDED(psResult))
    {
        if(errorBlob)
        {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
            errorBlob->Release();
        }

        vsBlob->Release();
        return(false);
    }

    RendererData.Device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &RendererData.VertexShader);
    RendererData.Device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &RendererData.PixelShader);

    D3D11_INPUT_ELEMENT_DESC inputElements[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    RendererData.Device->CreateInputLayout(inputElements, SSTL_ARRAYCOUNT(inputElements), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &RendererData.InputLayout);

    vsBlob->Release();
    psBlob->Release();

    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.ByteWidth = sizeof(Vertex) * 4;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    RendererData.Device->CreateBuffer(&vertexBufferDesc, nullptr, &RendererData.VertexBuffer);

    D3D11_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_NONE;

    RendererData.Device->CreateRasterizerState(&rasterizerDesc, &RendererData.RasterizerState);
    RendererData.Context->RSSetState(RendererData.RasterizerState);

    return(true);
}

void RenderDrawQuad(Quad quad)
{
    real32 left = ((real32)quad.Position[0] / (real32)RendererData.Width) * 2.0f - 1.0f;
    real32 right = ((real32)(quad.Position[0] + quad.Size[0]) / (real32)RendererData.Width) * 2.0f - 1.0f;
    real32 top = 1.0f - ((real32)quad.Position[1] / (real32)RendererData.Height) * 2.0f;
    real32 bottom = 1.0f - ((real32)(quad.Position[1] + quad.Size[1]) / (real32)RendererData.Height) * 2.0f;

    D3D11_MAPPED_SUBRESOURCE mapped;
    RendererData.Context->Map(RendererData.VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

    Vertex* vertices = (Vertex*)mapped.pData;
    vertices[0] = { { left, bottom }, { quad.Color.R, quad.Color.G, quad.Color.B, quad.Color.A } };
    vertices[1] = { { left, top }, { quad.Color.R, quad.Color.G, quad.Color.B, quad.Color.A } };
    vertices[2] = { { right, bottom }, { quad.Color.R, quad.Color.G, quad.Color.B, quad.Color.A } };
    vertices[3] = { { right, top }, { quad.Color.R, quad.Color.G, quad.Color.B, quad.Color.A } };

    RendererData.Context->Unmap(RendererData.VertexBuffer, 0);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    RendererData.Context->IASetVertexBuffers(0, 1, &RendererData.VertexBuffer, &stride, &offset);
    RendererData.Context->IASetInputLayout(RendererData.InputLayout);
    RendererData.Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    RendererData.Context->VSSetShader(RendererData.VertexShader, nullptr, 0);
    RendererData.Context->PSSetShader(RendererData.PixelShader, nullptr, 0);

    RendererData.Context->Draw(4, 0);
}

void Win32RenderClear(Color color)
{
    RendererData.RenderTargetView->Release();

    ID3D11Texture2D* backBuffer;
    RendererData.SwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));

    RendererData.Device->CreateRenderTargetView(backBuffer, nullptr, &RendererData.RenderTargetView);
    backBuffer->Release();

    RendererData.Context->OMSetRenderTargets(1, &RendererData.RenderTargetView, nullptr);

    real32 clearColor[4] = { color.R, color.G, color.B, color.A };
    RendererData.Context->ClearRenderTargetView(RendererData.RenderTargetView, clearColor);
}

void Win32RenderPresent()
{
    RendererData.SwapChain->Present(1, 0);
}

void Win32RenderShutdown()
{
    RendererData.RasterizerState->Release();
    RendererData.VertexBuffer->Release();
    RendererData.InputLayout->Release();
    RendererData.PixelShader->Release();
    RendererData.VertexShader->Release();
    RendererData.RenderTargetView->Release();
    RendererData.SwapChain->Release();
    RendererData.Context->Release();
    RendererData.Device->Release();
}
