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

static const char* UIPassShaderSource = R"HLSL(
cbuffer Constants : register(b0)
{
    row_major float4x4 Projection;
};

struct VSInput
{
    float2 Position : POSITION;
    float2 UV : TEXCOORD;
    float4 Color : COLOR;
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD;
    float4 Color : COLOR;
};

PSInput VSMain(VSInput input)
{
    PSInput output;
    output.Position = mul(float4(input.Position, 0.0f, 1.0f), Projection);
    output.UV = input.UV;
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

static void D3D11UIPassAppendQuad(D3D11RendererState* renderer, QuadKind kind, ID3D11ShaderResourceView* resolvedTexture, real32 x, real32 y, real32 w, real32 h, real32 u0, real32 v0, real32 u1, real32 v1, uint32 color)
{
    D3D11UIPass* uiPass = &renderer->UIPass;

    Assert(uiPass->QuadCount < uiPass->MaxQuads);
    if(uiPass->QuadCount >= uiPass->MaxQuads)
    {
        return;
    }

    // NOTE(saeb): Order-preserving batching: extend the current run, or open a new one when the texture or kind changes.
    if(uiPass->BatchCount == 0 || uiPass->Batches[uiPass->BatchCount - 1].Texture != resolvedTexture || uiPass->Batches[uiPass->BatchCount - 1].Kind != kind)
    {
        Assert(uiPass->BatchCount < MAX_UI_PASS_BATCHES);
        if(uiPass->BatchCount >= MAX_UI_PASS_BATCHES)
        {
            return;
        }

        TextureBatch* newBatch = &uiPass->Batches[uiPass->BatchCount++];
        newBatch->Texture = resolvedTexture;
        newBatch->Kind = kind;
        newBatch->QuadCount = 0;
    }

    // NOTE(saeb): Write the 4 corners in the order the index buffer expects: TL, TR, BL, BR.
    Vertex2D* quad = uiPass->Vertices + (uiPass->QuadCount * 4);

    real32 x1 = x + w;
    real32 y1 = y + h;

    quad[0].Position[0] = x;
    quad[0].Position[1] = y;
    quad[0].UV[0] = u0;
    quad[0].UV[1] = v0;
    quad[0].Color = color;

    quad[1].Position[0] = x1;
    quad[1].Position[1] = y;
    quad[1].UV[0] = u1;
    quad[1].UV[1] = v0;
    quad[1].Color = color;

    quad[2].Position[0] = x;
    quad[2].Position[1] = y1;
    quad[2].UV[0] = u0;
    quad[2].UV[1] = v1;
    quad[2].Color = color;

    quad[3].Position[0] = x1;
    quad[3].Position[1] = y1;
    quad[3].UV[0] = u1;
    quad[3].UV[1] = v1;
    quad[3].Color = color;

    uiPass->Batches[uiPass->BatchCount - 1].QuadCount++;
    uiPass->QuadCount++;
}

static void D3D11UIPassPushQuad(D3D11RendererState* renderer, real32 x, real32 y, real32 w, real32 h, real32 u0, real32 v0, real32 u1, real32 v1, ID3D11ShaderResourceView* texture, uint32 color)
{
    // NOTE(saeb): No texture means a solid-color quad reusing the 1x1 white texture.
    QuadKind kind = texture ? TEXTURED : SOLID;
    ID3D11ShaderResourceView* resolvedTexture = texture ? texture : renderer->UIPass.WhiteTexture;

    D3D11UIPassAppendQuad(renderer, kind, resolvedTexture, x, y, w, h, u0, v0, u1, v1, color);
}

static void D3D11UIPassPushGlyphQuad(D3D11RendererState* renderer, real32 x, real32 y, real32 w, real32 h, real32 u0, real32 v0, real32 u1, real32 v1, ID3D11ShaderResourceView* atlas, uint32 color)
{
    Assert(atlas != 0);
    D3D11UIPassAppendQuad(renderer, GLYPH, atlas, x, y, w, h, u0, v0, u1, v1, color);
}

static bool32 D3D11InitUIPass(D3D11RendererState* renderer, Arena* permanent, Arena* transient, uint32 maxQuads)
{
    D3D11UIPass* uiPass = &renderer->UIPass;

    // NOTE(saeb): We index with uint16, so 4 verts/quad must stay within the 65536 vertex ceiling.
    Assert(maxQuads <= 16384);

    uiPass->MaxQuads = maxQuads;
    uiPass->QuadCount = 0;
    uiPass->BatchCount = 0;
    uiPass->TextureRegistryCount = 1;

    uiPass->Vertices = PushArray(permanent, Vertex2D, maxQuads * 4);
    uiPass->Batches = PushArray(permanent, TextureBatch, MAX_UI_PASS_BATCHES);

    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.ByteWidth = maxQuads * 4 * sizeof(Vertex2D);
    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    if(FAILED(renderer->Device->CreateBuffer(&vertexBufferDesc, 0, &uiPass->VertexBuffer)))
    {
        return(false);
    }

    uint32 indexCount = maxQuads * 6;
    uint16* indices = PushArray(transient, uint16, indexCount);

    for(uint32 quadIndex = 0; quadIndex < maxQuads; ++quadIndex)
    {
        uint16 base = (uint16)(quadIndex * 4);
        uint32 i = quadIndex * 6;

        indices[i + 0] = base + 0; // Triangle 1: TL, TR, BL
        indices[i + 1] = base + 1;
        indices[i + 2] = base + 2;
        indices[i + 3] = base + 2; // Triangle 2: BL, TR, BR
        indices[i + 4] = base + 1;
        indices[i + 5] = base + 3;
    }

    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.ByteWidth = indexCount * sizeof(uint16);
    indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA indexBufferData = {};
    indexBufferData.pSysMem = indices;

    if(FAILED(renderer->Device->CreateBuffer(&indexBufferDesc, &indexBufferData, &uiPass->IndexBuffer)))
    {
        return(false);
    }

    // NOTE(saeb): This constant buffer holds the ortho matrix (4*4); updated on resize.
    D3D11_BUFFER_DESC constantBufferDesc = {};
    constantBufferDesc.ByteWidth = 16 * sizeof(real32);
    constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    if(FAILED(renderer->Device->CreateBuffer(&constantBufferDesc, 0, &uiPass->ConstantBuffer)))
    {
        return(false);
    }

    ID3DBlob* vertexShaderBlob;
    if(!D3D11CompileShader(UIPassShaderSource, "VSMain", "vs_5_0", &vertexShaderBlob))
    {
        return(false);
    }

    ID3DBlob* pixelShaderBlob;
    if(!D3D11CompileShader(UIPassShaderSource, "PSMain", "ps_5_0", &pixelShaderBlob))
    {
        vertexShaderBlob->Release();
        return(false);
    }

    ID3DBlob* pixelShaderAlphaBlob;
    if(!D3D11CompileShader(UIPassShaderSource, "PSMainAlpha", "ps_5_0", &pixelShaderAlphaBlob))
    {
        vertexShaderBlob->Release();
        pixelShaderBlob->Release();
        return(false);
    }

    HRESULT vertexShaderResult = renderer->Device->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), 0, &uiPass->VertexShader);
    HRESULT pixelShaderResult = renderer->Device->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), 0, &uiPass->PixelShader);
    HRESULT pixelShaderAlphaResult = renderer->Device->CreatePixelShader(pixelShaderAlphaBlob->GetBufferPointer(), pixelShaderAlphaBlob->GetBufferSize(), 0, &uiPass->PixelShaderAlpha);

    // NOTE(saeb): Input layout maps Vertex2D's bytes to the VS inputs, validated against its bytecode.
    D3D11_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex2D, Position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex2D, UV), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, offsetof(Vertex2D, Color), D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    HRESULT inputLayoutResult = renderer->Device->CreateInputLayout(inputLayout, ArrayCount(inputLayout), vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), &uiPass->InputLayout);

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

    if(FAILED(renderer->Device->CreateBlendState(&blendDesc, &uiPass->BlendState)))
    {
        return(false);
    }

    // NOTE(saeb): Depth is off - 2D draws on top of the 3D scene, no depth test or write.
    D3D11_DEPTH_STENCIL_DESC depthDesc = {};
    depthDesc.DepthEnable = FALSE;
    depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depthDesc.StencilEnable = FALSE;

    if(FAILED(renderer->Device->CreateDepthStencilState(&depthDesc, &uiPass->DepthState)))
    {
        return(false);
    }

    // NOTE(saeb): Rasterizer with no culling - 2D quads should never vanish due to winding.
    D3D11_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_NONE;
    rasterizerDesc.DepthClipEnable = TRUE;

    if(FAILED(renderer->Device->CreateRasterizerState(&rasterizerDesc, &uiPass->RasterizerState)))
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

    if(FAILED(renderer->Device->CreateSamplerState(&samplerDesc, &uiPass->Sampler)))
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

    HRESULT whiteViewResult = renderer->Device->CreateShaderResourceView(whiteTexture, 0, &uiPass->WhiteTexture);
    whiteTexture->Release();

    if(FAILED(whiteViewResult))
    {
        return(false);
    }

    return(true);
}

bool32 D3D11InitRenderer(D3D11RendererState* renderer, HWND windowHandle, int32 width, int32 height, Arena* permanent, Arena* transient, uint32 maxQuads)
{
    if(!D3D11InitDevice(renderer, windowHandle, width, height))
    {
        return(false);
    }

    if(!D3D11InitUIPass(renderer, permanent, transient, maxQuads))
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

    D3D11UIPass* uiPass = &renderer->UIPass;

    // NOTE(saeb): Release every texture handed to us via D3D11RegisterTexture (handle 0 is reserved/unused).
    for(uint32 i = 1; i < uiPass->TextureRegistryCount; ++i)
    {
        if(uiPass->TextureRegistry[i].Texture)
        {
            uiPass->TextureRegistry[i].Texture->Release();
            uiPass->TextureRegistry[i].Texture = 0;
        }
    }

    if(uiPass->WhiteTexture)
    {
        uiPass->WhiteTexture->Release();
        uiPass->WhiteTexture = 0;
    }

    if(uiPass->Sampler)
    {
        uiPass->Sampler->Release();
        uiPass->Sampler = 0;
    }

    if(uiPass->RasterizerState)
    {
        uiPass->RasterizerState->Release();
        uiPass->RasterizerState = 0;
    }

    if(uiPass->DepthState)
    {
        uiPass->DepthState->Release();
        uiPass->DepthState = 0;
    }

    if(uiPass->BlendState)
    {
        uiPass->BlendState->Release();
        uiPass->BlendState = 0;
    }

    if(uiPass->InputLayout)
    {
        uiPass->InputLayout->Release();
        uiPass->InputLayout = 0;
    }

    if(uiPass->PixelShaderAlpha)
    {
        uiPass->PixelShaderAlpha->Release();
        uiPass->PixelShaderAlpha = 0;
    }
    
    if(uiPass->PixelShader)
    {
        uiPass->PixelShader->Release();
        uiPass->PixelShader = 0;
    }
    
    if(uiPass->VertexShader)
    {
        uiPass->VertexShader->Release();
        uiPass->VertexShader = 0;
    }
    
    if(uiPass->ConstantBuffer)
    {
        uiPass->ConstantBuffer->Release();
        uiPass->ConstantBuffer = 0;
    }
    
    if(uiPass->IndexBuffer)
    {
        uiPass->IndexBuffer->Release();
        uiPass->IndexBuffer = 0;
    }
    
    if(uiPass->VertexBuffer)
    {
        uiPass->VertexBuffer->Release();
        uiPass->VertexBuffer = 0;
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

void D3D11BeginUIPass(D3D11RendererState* renderer, int32 width, int32 height)
{
    D3D11UIPass* uiPass = &renderer->UIPass;

    uiPass->QuadCount = 0;
    uiPass->BatchCount = 0;

    // NOTE(saeb): Rebuild + upload the projection only when the size actually changed.
    if(width != uiPass->ViewportWidth || height != uiPass->ViewportHeight)
    {
        uiPass->ViewportWidth = width;
        uiPass->ViewportHeight = height;

        DirectX::XMMATRIX projection = DirectX::XMMatrixOrthographicOffCenterLH(0.0f, (real32)width, (real32)height, 0.0f, 0.0f, 1.0f);

        D3D11_MAPPED_SUBRESOURCE mapped;
        if(SUCCEEDED(renderer->Context->Map(uiPass->ConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
        {
            memcpy(mapped.pData, &projection, sizeof(projection)); // Row-major, no transpose
            renderer->Context->Unmap(uiPass->ConstantBuffer, 0);
        }
    }
}

uint32 D3D11RegisterTexture(D3D11RendererState* renderer, ID3D11ShaderResourceView* texture, QuadKind kind)
{
    D3D11UIPass* uiPass = &renderer->UIPass;

    Assert(uiPass->TextureRegistryCount < MAX_TEXTURE_HANDLES);
    if(uiPass->TextureRegistryCount >= MAX_TEXTURE_HANDLES)
    {
        return(0);
    }

    uint32 handle = uiPass->TextureRegistryCount++;
    uiPass->TextureRegistry[handle].Texture = texture;
    uiPass->TextureRegistry[handle].Kind = kind;
    return(handle);
}

void D3D11SubmitRenderCommands(D3D11RendererState* renderer, RenderCommands* commands)
{
    D3D11UIPass* uiPass = &renderer->UIPass;

    for(uint32 i = 0; i < commands->QuadCount; ++i)
    {
        RenderCommandQuad* quad = &commands->Quads[i];

        if(quad->TextureHandle == 0)
        {
            D3D11UIPassPushQuad(renderer, quad->X, quad->Y, quad->Width, quad->Height, quad->U0, quad->V0, quad->U1, quad->V1, 0, quad->Color);
            continue;
        }

        TextureRegistryEntry* entry = &uiPass->TextureRegistry[quad->TextureHandle];

        if(entry->Kind == GLYPH)
        {
            D3D11UIPassPushGlyphQuad(renderer, quad->X, quad->Y, quad->Width, quad->Height, quad->U0, quad->V0, quad->U1, quad->V1, entry->Texture, quad->Color);
        }
        else
        {
            D3D11UIPassPushQuad(renderer, quad->X, quad->Y, quad->Width, quad->Height, quad->U0, quad->V0, quad->U1, quad->V1, entry->Texture, quad->Color);
        }
    }
}

void D3D11EndUIPass(D3D11RendererState* renderer)
{
    D3D11UIPass* uiPass = &renderer->UIPass;

    if(uiPass->QuadCount == 0)
    {
        return;
    }

    D3D11_MAPPED_SUBRESOURCE mapped;
    if(FAILED(renderer->Context->Map(uiPass->VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
    {
        return;
    }

    memcpy(mapped.pData, uiPass->Vertices, uiPass->QuadCount * 4 * sizeof(Vertex2D));
    renderer->Context->Unmap(uiPass->VertexBuffer, 0);

    UINT stride = sizeof(Vertex2D);
    UINT offset = 0;

    renderer->Context->IASetInputLayout(uiPass->InputLayout);
    renderer->Context->IASetVertexBuffers(0, 1, &uiPass->VertexBuffer, &stride, &offset);
    renderer->Context->IASetIndexBuffer(uiPass->IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    renderer->Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    renderer->Context->VSSetShader(uiPass->VertexShader, 0, 0);
    renderer->Context->VSSetConstantBuffers(0, 1, &uiPass->ConstantBuffer);

    renderer->Context->PSSetSamplers(0, 1, &uiPass->Sampler);

    renderer->Context->RSSetState(uiPass->RasterizerState);
    renderer->Context->OMSetBlendState(uiPass->BlendState, 0, 0xFFFFFFFF);
    renderer->Context->OMSetDepthStencilState(uiPass->DepthState, 0);

    UINT indexOffset = 0;
    for(uint32 batchIndex = 0; batchIndex < uiPass->BatchCount; ++batchIndex)
    {
        TextureBatch* batch = &uiPass->Batches[batchIndex];
        UINT indexCount = batch->QuadCount * 6;

        ID3D11PixelShader* pixelShader = (batch->Kind == GLYPH) ? uiPass->PixelShaderAlpha : uiPass->PixelShader;
        renderer->Context->PSSetShader(pixelShader, 0, 0);
        renderer->Context->PSSetShaderResources(0, 1, &batch->Texture);
        renderer->Context->DrawIndexed(indexCount, indexOffset, 0);

        indexOffset += indexCount;
    }
}
