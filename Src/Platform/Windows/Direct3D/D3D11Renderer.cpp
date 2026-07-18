#include "D3D11Renderer.h"
#include "Engine/RenderCommands.h"

#include <string.h>

#include <d3dcompiler.h>
#include <directxmath.h>

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
    // NOTE(saeb): Ask DXGI for adapters in preference order. DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE respects the user's per-app GPU choice and otherwise favors the discrete GPU, so we just take the first hardware adapter it returns.
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

static bool32 D3D11InitDevice(D3D11RendererState* renderer, HWND windowHandle, int32 width, int32 height)
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

static const char* QuadPassShaderSource = R"HLSL(
cbuffer Constants : register(b0)
{
    row_major float4x4 Projection;
};

struct VSInput
{
    float2 Corner : POSITION;   // Per-vertex: unit quad corner
    float4 Rect   : RECT;       // Per-instance: x, y, width, height
    float4 UVRect : UVRECT;     // Per-instance: u0, v0, u1, v1
    float4 Color  : COLOR;      // Per-instance
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD;
    float4 Color : COLOR;
};

PSInput VSMain(VSInput input)
{
    float2 position = input.Rect.xy + input.Corner * input.Rect.zw;
    float2 uv = lerp(input.UVRect.xy, input.UVRect.zw, input.Corner);

    PSInput output;
    output.Position = mul(float4(position, 0.0f, 1.0f), Projection);
    output.UV = uv;
    output.Color = input.Color;

    return output;
}

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

float4 PSMain(PSInput input) : SV_TARGET
{
    return Texture.Sample(Sampler, input.UV) * input.Color;
}

float4 PSMainAlpha(PSInput input) : SV_TARGET
{
    float coverage = Texture.Sample(Sampler, input.UV).r;
    return float4(1.0f, 1.0f, 1.0f, coverage) * input.Color;
}
)HLSL";

static bool32 D3D11CompileShader(const char* source, const char* entryPoint, const char* target, ID3DBlob** outBlob)
{
    UINT flags = 0;

#if defined(AG_DEBUG)
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ID3DBlob* errorBlob = 0;
    HRESULT result = D3DCompile(source, strlen(source), 0, 0, 0, entryPoint, target, flags, 0, outBlob, &errorBlob);

    if(errorBlob)
    {
        // TODO(saeb): Logging.
        errorBlob->Release();
    }

    return(SUCCEEDED(result));
}

static void D3D11QuadPassAppendQuad(D3D11RendererState* renderer, QuadKind kind, ID3D11ShaderResourceView* resolvedTexture, real32 x, real32 y, real32 w, real32 h, real32 u0, real32 v0, real32 u1, real32 v1, uint32 color)
{
    D3D11QuadPass* quadPass = &renderer->QuadPass;

    Assert(quadPass->InstanceCount < quadPass->MaxQuads);
    if(quadPass->InstanceCount >= quadPass->MaxQuads)
    {
        return;
    }

    // NOTE(saeb): Order-preserving batching: extend the current run, or open a new one when the texture or kind changes.
    if(quadPass->BatchCount == 0 || quadPass->Batches[quadPass->BatchCount - 1].Texture != resolvedTexture || quadPass->Batches[quadPass->BatchCount - 1].Kind != kind)
    {
        Assert(quadPass->BatchCount < MAX_QUAD_PASS_BATCHES);
        if(quadPass->BatchCount >= MAX_QUAD_PASS_BATCHES)
        {
            return;
        }

        TextureBatch* newBatch = &quadPass->Batches[quadPass->BatchCount++];
        newBatch->Texture = resolvedTexture;
        newBatch->Kind = kind;
        newBatch->InstanceCount = 0;
    }

    // NOTE(saeb): One record per quad; the vertex shader expands it against the shared unit quad.
    QuadInstance* instance = quadPass->Instances + quadPass->InstanceCount;

    instance->Rect[0] = x;
    instance->Rect[1] = y;
    instance->Rect[2] = w;
    instance->Rect[3] = h;

    instance->UV[0] = u0;
    instance->UV[1] = v0;
    instance->UV[2] = u1;
    instance->UV[3] = v1;

    instance->Color = color;

    quadPass->Batches[quadPass->BatchCount - 1].InstanceCount++;
    quadPass->InstanceCount++;
}

static void D3D11QuadPassPushQuad(D3D11RendererState* renderer, real32 x, real32 y, real32 w, real32 h, real32 u0, real32 v0, real32 u1, real32 v1, ID3D11ShaderResourceView* texture, uint32 color)
{
    // NOTE(saeb): No texture means a solid-color quad reusing the 1x1 white texture.
    QuadKind kind = texture ? TEXTURED : SOLID;
    ID3D11ShaderResourceView* resolvedTexture = texture ? texture : renderer->QuadPass.WhiteTexture;

    D3D11QuadPassAppendQuad(renderer, kind, resolvedTexture, x, y, w, h, u0, v0, u1, v1, color);
}

static void D3D11QuadPassPushGlyphQuad(D3D11RendererState* renderer, real32 x, real32 y, real32 w, real32 h, real32 u0, real32 v0, real32 u1, real32 v1, ID3D11ShaderResourceView* atlas, uint32 color)
{
    Assert(atlas != 0);
    D3D11QuadPassAppendQuad(renderer, GLYPH, atlas, x, y, w, h, u0, v0, u1, v1, color);
}

static bool32 D3D11InitQuadPass(D3D11RendererState* renderer, Arena* permanent, uint32 maxQuads)
{
    D3D11QuadPass* quadPass = &renderer->QuadPass;

    quadPass->MaxQuads = maxQuads;
    quadPass->InstanceCount = 0;
    quadPass->BatchCount = 0;
    quadPass->TextureRegistryCount = 1;

    quadPass->Instances = PushArray(permanent, QuadInstance, maxQuads);
    quadPass->Batches = PushArray(permanent, TextureBatch, MAX_QUAD_PASS_BATCHES);

    // NOTE(saeb): Shared unit quad - every quad is this geometry expanded by its instance rect.
    static const QuadVertex unitQuad[] =
        {
            { { 0.0f, 0.0f } },
            { { 1.0f, 0.0f } },
            { { 0.0f, 1.0f } },
            { { 1.0f, 1.0f } }
        };
    static const uint16 unitIndices[] = { 0, 1, 2, 2, 1, 3 }; // Triangle 1: TL, TR, BL - Triangle 2: BL, TR, BR

    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.ByteWidth = sizeof(unitQuad);
    vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexBufferData = {};
    vertexBufferData.pSysMem = unitQuad;

    if(FAILED(renderer->Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &quadPass->UnitQuadBuffer)))
    {
        return(false);
    }

    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.ByteWidth = sizeof(unitIndices);
    indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA indexBufferData = {};
    indexBufferData.pSysMem = unitIndices;

    if(FAILED(renderer->Device->CreateBuffer(&indexBufferDesc, &indexBufferData, &quadPass->IndexBuffer)))
    {
        return(false);
    }

    // NOTE(saeb): Per-quad instance data, rewritten every frame.
    D3D11_BUFFER_DESC instanceBufferDesc = {};
    instanceBufferDesc.ByteWidth = maxQuads * sizeof(QuadInstance);
    instanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    instanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    if(FAILED(renderer->Device->CreateBuffer(&instanceBufferDesc, 0, &quadPass->InstanceBuffer)))
    {
        return(false);
    }

    // NOTE(saeb): This constant buffer holds the ortho matrix (4*4); updated on resize.
    D3D11_BUFFER_DESC constantBufferDesc = {};
    constantBufferDesc.ByteWidth = 16 * sizeof(real32);
    constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    if(FAILED(renderer->Device->CreateBuffer(&constantBufferDesc, 0, &quadPass->ConstantBuffer)))
    {
        return(false);
    }

    ID3DBlob* vertexShaderBlob;
    if(!D3D11CompileShader(QuadPassShaderSource, "VSMain", "vs_5_0", &vertexShaderBlob))
    {
        return(false);
    }

    ID3DBlob* pixelShaderBlob;
    if(!D3D11CompileShader(QuadPassShaderSource, "PSMain", "ps_5_0", &pixelShaderBlob))
    {
        vertexShaderBlob->Release();
        return(false);
    }

    ID3DBlob* pixelShaderAlphaBlob;
    if(!D3D11CompileShader(QuadPassShaderSource, "PSMainAlpha", "ps_5_0", &pixelShaderAlphaBlob))
    {
        vertexShaderBlob->Release();
        pixelShaderBlob->Release();
        return(false);
    }

    HRESULT vertexShaderResult = renderer->Device->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), 0, &quadPass->VertexShader);
    HRESULT pixelShaderResult = renderer->Device->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), 0, &quadPass->PixelShader);
    HRESULT pixelShaderAlphaResult = renderer->Device->CreatePixelShader(pixelShaderAlphaBlob->GetBufferPointer(), pixelShaderAlphaBlob->GetBufferSize(), 0, &quadPass->PixelShaderAlpha);

    // NOTE(saeb): Slot 0 is the shared unit quad (per-vertex); slot 1 is per-instance quad data.
    D3D11_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(QuadVertex, Corner), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "RECT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, offsetof(QuadInstance, Rect), D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "UVRECT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, offsetof(QuadInstance, UV), D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 1, offsetof(QuadInstance, Color), D3D11_INPUT_PER_INSTANCE_DATA, 1 }
    };

    HRESULT inputLayoutResult = renderer->Device->CreateInputLayout(inputLayout, ArrayCount(inputLayout), vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), &quadPass->InputLayout);

    vertexShaderBlob->Release();
    pixelShaderBlob->Release();
    pixelShaderAlphaBlob->Release();

    if(FAILED(vertexShaderResult) || FAILED(pixelShaderResult) || FAILED(pixelShaderAlphaResult) || FAILED(inputLayoutResult))
    {
        return(false);
    }

    // NOTE(saeb): Blend is straight alpha - out = src*src.a + dst*(1-src.a).
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    if(FAILED(renderer->Device->CreateBlendState(&blendDesc, &quadPass->BlendState)))
    {
        return(false);
    }

    // NOTE(saeb): Depth is off - 2D draws on top of the 3D scene, no depth test or write.
    D3D11_DEPTH_STENCIL_DESC depthDesc = {};
    depthDesc.DepthEnable = FALSE;
    depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depthDesc.StencilEnable = FALSE;

    if(FAILED(renderer->Device->CreateDepthStencilState(&depthDesc, &quadPass->DepthState)))
    {
        return(false);
    }

    // NOTE(saeb): Rasterizer with no culling - 2D quads should never vanish due to winding.
    D3D11_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_NONE;
    rasterizerDesc.DepthClipEnable = TRUE;

    if(FAILED(renderer->Device->CreateRasterizerState(&rasterizerDesc, &quadPass->RasterizerState)))
    {
        return(false);
    }

    // NOTE(saeb): Sampler with linear filter, clamp (atlases don't tile; clamp avoids edge bleed).
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    if(FAILED(renderer->Device->CreateSamplerState(&samplerDesc, &quadPass->Sampler)))
    {
        return(false);
    }

    uint32 whitePixel = 0xFFFFFFFF;

    D3D11_TEXTURE2D_DESC whiteDesc = {};
    whiteDesc.Width = 1;
    whiteDesc.Height = 1;
    whiteDesc.MipLevels = 1;
    whiteDesc.ArraySize = 1;
    whiteDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    whiteDesc.SampleDesc.Count = 1;
    whiteDesc.Usage = D3D11_USAGE_IMMUTABLE;
    whiteDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA whiteData = {};
    whiteData.pSysMem = &whitePixel;
    whiteData.SysMemPitch = sizeof(whitePixel);

    ID3D11Texture2D* whiteTexture;
    if(FAILED(renderer->Device->CreateTexture2D(&whiteDesc, &whiteData, &whiteTexture)))
    {
        return(false);
    }

    HRESULT whiteViewResult = renderer->Device->CreateShaderResourceView(whiteTexture, 0, &quadPass->WhiteTexture);
    whiteTexture->Release();

    if(FAILED(whiteViewResult))
    {
        return(false);
    }

    return(true);
}

bool32 D3D11InitRenderer(D3D11RendererState* renderer, HWND windowHandle, int32 width, int32 height, Arena* permanent, uint32 maxQuads)
{
    if(!D3D11InitDevice(renderer, windowHandle, width, height))
    {
        return(false);
    }

    if(!D3D11InitQuadPass(renderer, permanent, maxQuads))
    {
        return(false);
    }

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

    D3D11QuadPass* quadPass = &renderer->QuadPass;

    // NOTE(saeb): Release every texture handed to us via D3D11RegisterTexture (handle 0 is reserved/unused).
    for(uint32 i = 1; i < quadPass->TextureRegistryCount; ++i)
    {
        if(quadPass->TextureRegistry[i].Texture)
        {
            quadPass->TextureRegistry[i].Texture->Release();
            quadPass->TextureRegistry[i].Texture = 0;
        }
    }

    if(quadPass->WhiteTexture)
    {
        quadPass->WhiteTexture->Release();
        quadPass->WhiteTexture = 0;
    }

    if(quadPass->Sampler)
    {
        quadPass->Sampler->Release();
        quadPass->Sampler = 0;
    }

    if(quadPass->RasterizerState)
    {
        quadPass->RasterizerState->Release();
        quadPass->RasterizerState = 0;
    }

    if(quadPass->DepthState)
    {
        quadPass->DepthState->Release();
        quadPass->DepthState = 0;
    }

    if(quadPass->BlendState)
    {
        quadPass->BlendState->Release();
        quadPass->BlendState = 0;
    }

    if(quadPass->InputLayout)
    {
        quadPass->InputLayout->Release();
        quadPass->InputLayout = 0;
    }

    if(quadPass->PixelShaderAlpha)
    {
        quadPass->PixelShaderAlpha->Release();
        quadPass->PixelShaderAlpha = 0;
    }
    
    if(quadPass->PixelShader)
    {
        quadPass->PixelShader->Release();
        quadPass->PixelShader = 0;
    }
    
    if(quadPass->VertexShader)
    {
        quadPass->VertexShader->Release();
        quadPass->VertexShader = 0;
    }
    
    if(quadPass->ConstantBuffer)
    {
        quadPass->ConstantBuffer->Release();
        quadPass->ConstantBuffer = 0;
    }
    
    if(quadPass->IndexBuffer)
    {
        quadPass->IndexBuffer->Release();
        quadPass->IndexBuffer = 0;
    }

    if(quadPass->InstanceBuffer)
    {
        quadPass->InstanceBuffer->Release();
        quadPass->InstanceBuffer = 0;
    }

    if(quadPass->UnitQuadBuffer)
    {
        quadPass->UnitQuadBuffer->Release();
        quadPass->UnitQuadBuffer = 0;
    }

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

void D3D11BeginQuadPass(D3D11RendererState* renderer, int32 width, int32 height)
{
    D3D11QuadPass* quadPass = &renderer->QuadPass;

    quadPass->InstanceCount = 0;
    quadPass->BatchCount = 0;

    // NOTE(saeb): Rebuild + upload the projection only when the size actually changed.
    if(width != quadPass->ViewportWidth || height != quadPass->ViewportHeight)
    {
        quadPass->ViewportWidth = width;
        quadPass->ViewportHeight = height;

        DirectX::XMMATRIX projection = DirectX::XMMatrixOrthographicOffCenterLH(0.0f, (real32)width, (real32)height, 0.0f, 0.0f, 1.0f);

        D3D11_MAPPED_SUBRESOURCE mapped;
        if(SUCCEEDED(renderer->Context->Map(quadPass->ConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
        {
            memcpy(mapped.pData, &projection, sizeof(projection)); // Row-major, no transpose
            renderer->Context->Unmap(quadPass->ConstantBuffer, 0);
        }
    }
}

uint32 D3D11RegisterTexture(D3D11RendererState* renderer, ID3D11ShaderResourceView* texture, QuadKind kind)
{
    D3D11QuadPass* quadPass = &renderer->QuadPass;

    Assert(quadPass->TextureRegistryCount < MAX_TEXTURE_HANDLES);
    if(quadPass->TextureRegistryCount >= MAX_TEXTURE_HANDLES)
    {
        return(0);
    }

    uint32 handle = quadPass->TextureRegistryCount++;
    quadPass->TextureRegistry[handle].Texture = texture;
    quadPass->TextureRegistry[handle].Kind = kind;
    return(handle);
}

void D3D11SubmitRenderCommands(D3D11RendererState* renderer, RenderCommands* commands)
{
    D3D11QuadPass* quadPass = &renderer->QuadPass;

    for(uint32 i = 0; i < commands->QuadCount; ++i)
    {
        RenderCommandQuad* quad = &commands->Quads[i];

        if(quad->TextureHandle == 0)
        {
            D3D11QuadPassPushQuad(renderer, quad->X, quad->Y, quad->Width, quad->Height, quad->U0, quad->V0, quad->U1, quad->V1, 0, quad->Color);
            continue;
        }

        TextureRegistryEntry* entry = &quadPass->TextureRegistry[quad->TextureHandle];

        if(entry->Kind == GLYPH)
        {
            D3D11QuadPassPushGlyphQuad(renderer, quad->X, quad->Y, quad->Width, quad->Height, quad->U0, quad->V0, quad->U1, quad->V1, entry->Texture, quad->Color);
        }
        else
        {
            D3D11QuadPassPushQuad(renderer, quad->X, quad->Y, quad->Width, quad->Height, quad->U0, quad->V0, quad->U1, quad->V1, entry->Texture, quad->Color);
        }
    }
}

void D3D11EndQuadPass(D3D11RendererState* renderer)
{
    D3D11QuadPass* quadPass = &renderer->QuadPass;

    if(quadPass->InstanceCount == 0)
    {
        return;
    }

    D3D11_MAPPED_SUBRESOURCE mapped;
    if(FAILED(renderer->Context->Map(quadPass->InstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
    {
        return;
    }

    memcpy(mapped.pData, quadPass->Instances, quadPass->InstanceCount * sizeof(QuadInstance));
    renderer->Context->Unmap(quadPass->InstanceBuffer, 0);

    ID3D11Buffer* vertexBuffers[2] = { quadPass->UnitQuadBuffer, quadPass->InstanceBuffer };
    UINT strides[2] = { sizeof(QuadVertex), sizeof(QuadInstance) };
    UINT offsets[2] = { 0, 0 };

    renderer->Context->IASetInputLayout(quadPass->InputLayout);
    renderer->Context->IASetVertexBuffers(0, 2, vertexBuffers, strides, offsets);
    renderer->Context->IASetIndexBuffer(quadPass->IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    renderer->Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    renderer->Context->VSSetShader(quadPass->VertexShader, 0, 0);
    renderer->Context->VSSetConstantBuffers(0, 1, &quadPass->ConstantBuffer);

    renderer->Context->PSSetSamplers(0, 1, &quadPass->Sampler);

    renderer->Context->RSSetState(quadPass->RasterizerState);
    renderer->Context->OMSetBlendState(quadPass->BlendState, 0, 0xFFFFFFFF);
    renderer->Context->OMSetDepthStencilState(quadPass->DepthState, 0);

    UINT instanceOffset = 0;
    for(uint32 batchIndex = 0; batchIndex < quadPass->BatchCount; ++batchIndex)
    {
        TextureBatch* batch = &quadPass->Batches[batchIndex];

        ID3D11PixelShader* pixelShader = (batch->Kind == GLYPH) ? quadPass->PixelShaderAlpha : quadPass->PixelShader;
        renderer->Context->PSSetShader(pixelShader, 0, 0);
        renderer->Context->PSSetShaderResources(0, 1, &batch->Texture);
        renderer->Context->DrawIndexedInstanced(6, batch->InstanceCount, 0, 0, instanceOffset);

        instanceOffset += batch->InstanceCount;
    }
}
