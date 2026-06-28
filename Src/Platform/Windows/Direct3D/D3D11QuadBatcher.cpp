#include "D3D11QuadBatcher.h"
#include "D3D11Renderer.h"
#include "Engine/RenderCommands.h"

#include <string.h>

#include <d3dcompiler.h>
#include <directxmath.h>

static const char* QuadShaderSource = R"HLSL(
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

struct TextureBatch
{
    QuadKind Kind;
    ID3D11ShaderResourceView* Texture;
    uint32 QuadCount;
};

struct TextureRegistryEntry
{
    ID3D11ShaderResourceView* Texture;
    QuadKind Kind;
};

struct D3D11QuadBatcherState
{
    // CPU-side accumulation.
    Vertex2D* Vertices;
    TextureBatch* Batches;
    uint32 MaxQuads;
    uint32 QuadCount;
    uint32 BatchCount;

    // GPU resources.
    ID3D11Device* Device;
    ID3D11DeviceContext* Context;
    ID3D11Buffer* VertexBuffer;
    ID3D11Buffer* IndexBuffer;
    ID3D11Buffer* ConstantBuffer;
    ID3D11VertexShader* VertexShader;
    ID3D11PixelShader* PixelShader;
    ID3D11PixelShader* PixelShaderAlpha;
    ID3D11InputLayout* InputLayout;
    ID3D11BlendState* BlendState;
    ID3D11DepthStencilState* DepthState;
    ID3D11RasterizerState* RasterizerState;
    ID3D11SamplerState* Sampler;
    ID3D11ShaderResourceView* WhiteTexture;

    int32 ViewportWidth;
    int32 ViewportHeight;

    TextureRegistryEntry TextureRegistry[MAX_TEXTURE_HANDLES];
    uint32 TextureRegistryCount;
};
static D3D11QuadBatcherState QuadBatcher;

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

static void D3D11QuadBatcherAppendQuad(QuadKind kind, ID3D11ShaderResourceView* resolvedTexture, real32 x, real32 y, real32 w, real32 h, real32 u0, real32 v0, real32 u1, real32 v1, uint32 color)
{
    Assert(QuadBatcher.QuadCount < QuadBatcher.MaxQuads);
    if(QuadBatcher.QuadCount >= QuadBatcher.MaxQuads)
    {
        return;
    }

    // NOTE(saeb): Order-preserving batching: extend the current run, or open a new one when the texture or kind changes.
    if(QuadBatcher.BatchCount == 0 || QuadBatcher.Batches[QuadBatcher.BatchCount - 1].Texture != resolvedTexture || QuadBatcher.Batches[QuadBatcher.BatchCount - 1].Kind != kind)
    {
        Assert(QuadBatcher.BatchCount < MAX_TEXTURE_BATCHES);
        if(QuadBatcher.BatchCount >= MAX_TEXTURE_BATCHES)
        {
            return;
        }

        TextureBatch* newBatch = &QuadBatcher.Batches[QuadBatcher.BatchCount++];
        newBatch->Texture = resolvedTexture;
        newBatch->Kind = kind;
        newBatch->QuadCount = 0;
    }

    // NOTE(saeb): Write the 4 corners in the order the index buffer expects: TL, TR, BL, BR.
    Vertex2D* quad = QuadBatcher.Vertices + (QuadBatcher.QuadCount * 4);

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

    QuadBatcher.Batches[QuadBatcher.BatchCount - 1].QuadCount++;
    QuadBatcher.QuadCount++;
}

bool32 D3D11InitQuadBatcher(D3D11RendererState* renderer, MemoryArena* permanent, MemoryArena* transient, uint32 maxQuads)
{
    QuadBatcher.Device = renderer->Device;
    QuadBatcher.Context = renderer->Context;

    // NOTE(saeb): We index with uint16, so 4 verts/quad must stay within the 65536 vertex ceiling.
    Assert(maxQuads <= 16384);

    QuadBatcher.MaxQuads = maxQuads;
    QuadBatcher.QuadCount = 0;
    QuadBatcher.BatchCount = 0;
    QuadBatcher.TextureRegistryCount = 1;

    QuadBatcher.Vertices = PushArray(permanent, Vertex2D, maxQuads * 4);
    QuadBatcher.Batches = PushArray(permanent, TextureBatch, MAX_TEXTURE_BATCHES);

    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.ByteWidth = maxQuads * 4 * sizeof(Vertex2D);
    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    if(FAILED(QuadBatcher.Device->CreateBuffer(&vertexBufferDesc, 0, &QuadBatcher.VertexBuffer)))
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

    if(FAILED(QuadBatcher.Device->CreateBuffer(&indexBufferDesc, &indexBufferData, &QuadBatcher.IndexBuffer)))
    {
        return(false);
    }

    // NOTE(saeb): This constant buffer holds the ortho matrix (4*4); updated on resize.
    D3D11_BUFFER_DESC constantBufferDesc = {};
    constantBufferDesc.ByteWidth = 16 * sizeof(real32);
    constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    if(FAILED(QuadBatcher.Device->CreateBuffer(&constantBufferDesc, 0, &QuadBatcher.ConstantBuffer)))
    {
        return(false);
    }

    ID3DBlob* vertexShaderBlob;
    if(!D3D11CompileShader(QuadShaderSource, "VSMain", "vs_5_0", &vertexShaderBlob))
    {
        return(false);
    }

    ID3DBlob* pixelShaderBlob;
    if(!D3D11CompileShader(QuadShaderSource, "PSMain", "ps_5_0", &pixelShaderBlob))
    {
        vertexShaderBlob->Release();
        return(false);
    }

    ID3DBlob* pixelShaderAlphaBlob;
    if(!D3D11CompileShader(QuadShaderSource, "PSMainAlpha", "ps_5_0", &pixelShaderAlphaBlob))
    {
        vertexShaderBlob->Release();
        pixelShaderBlob->Release();
        return(false);
    }

    HRESULT vertexShaderResult = QuadBatcher.Device->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), 0, &QuadBatcher.VertexShader);
    HRESULT pixelShaderResult = QuadBatcher.Device->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), 0, &QuadBatcher.PixelShader);
    HRESULT pixelShaderAlphaResult = QuadBatcher.Device->CreatePixelShader(pixelShaderAlphaBlob->GetBufferPointer(), pixelShaderAlphaBlob->GetBufferSize(), 0, &QuadBatcher.PixelShaderAlpha);

    // NOTE(saeb): Input layout maps Vertex2D's bytes to the VS inputs, validated against its bytecode.
    D3D11_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex2D, Position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex2D, UV), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, offsetof(Vertex2D, Color), D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    HRESULT inputLayoutResult = QuadBatcher.Device->CreateInputLayout(inputLayout, ArrayCount(inputLayout), vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), &QuadBatcher.InputLayout);

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

    if(FAILED(QuadBatcher.Device->CreateBlendState(&blendDesc, &QuadBatcher.BlendState)))
    {
        return(false);
    }

    // NOTE(saeb): Depth is off - 2D draws on top of the 3D scene, no depth test or write.
    D3D11_DEPTH_STENCIL_DESC depthDesc = {};
    depthDesc.DepthEnable = FALSE;
    depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depthDesc.StencilEnable = FALSE;

    if(FAILED(QuadBatcher.Device->CreateDepthStencilState(&depthDesc, &QuadBatcher.DepthState)))
    {
        return(false);
    }

    // NOTE(saeb): Rasterizer with no culling - 2D quads should never vanish due to winding.
    D3D11_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_NONE;
    rasterizerDesc.DepthClipEnable = TRUE;

    if(FAILED(QuadBatcher.Device->CreateRasterizerState(&rasterizerDesc, &QuadBatcher.RasterizerState)))
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

    if(FAILED(QuadBatcher.Device->CreateSamplerState(&samplerDesc, &QuadBatcher.Sampler)))
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
    if(FAILED(QuadBatcher.Device->CreateTexture2D(&whiteDesc, &whiteData, &whiteTexture)))
    {
        return(false);
    }

    HRESULT whiteViewResult = QuadBatcher.Device->CreateShaderResourceView(whiteTexture, 0, &QuadBatcher.WhiteTexture);
    whiteTexture->Release();

    if(FAILED(whiteViewResult))
    {
        return(false);
    }

    return(true);
}

void D3D11QuadBatcherBegin(int32 width, int32 height)
{
    QuadBatcher.QuadCount = 0;
    QuadBatcher.BatchCount = 0;

    // NOTE(saeb): Rebuild + upload the projection only when the size actually changed.
    if(width != QuadBatcher.ViewportWidth || height != QuadBatcher.ViewportHeight)
    {
        QuadBatcher.ViewportWidth = width;
        QuadBatcher.ViewportHeight = height;

        DirectX::XMMATRIX projection = DirectX::XMMatrixOrthographicOffCenterLH(0.0f, (real32)width, (real32)height, 0.0f, 0.0f, 1.0f);

        D3D11_MAPPED_SUBRESOURCE mapped;
        if(SUCCEEDED(QuadBatcher.Context->Map(QuadBatcher.ConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
        {
            memcpy(mapped.pData, &projection, sizeof(projection)); // Row-major, no transpose
            QuadBatcher.Context->Unmap(QuadBatcher.ConstantBuffer, 0);
        }
    }
}

void D3D11QuadBatcherPushQuad(real32 x, real32 y, real32 w, real32 h, real32 u0, real32 v0, real32 u1, real32 v1, ID3D11ShaderResourceView* texture, uint32 color)
{
    // NOTE(saeb): No texture means a solid-color quad reusing the 1x1 white texture.
    QuadKind kind = texture ? TEXTURED : SOLID;
    ID3D11ShaderResourceView* resolvedTexture = texture ? texture : QuadBatcher.WhiteTexture;

    D3D11QuadBatcherAppendQuad(kind, resolvedTexture, x, y, w, h, u0, v0, u1, v1, color);
}

void D3D11QuadBatcherPushGlyphQuad(real32 x, real32 y, real32 w, real32 h, real32 u0, real32 v0, real32 u1, real32 v1, ID3D11ShaderResourceView* atlas, uint32 color)
{
    Assert(atlas != 0);
    D3D11QuadBatcherAppendQuad(GLYPH, atlas, x, y, w, h, u0, v0, u1, v1, color);
}

uint32 D3D11QuadBatcherRegisterTexture(ID3D11ShaderResourceView* texture, QuadKind kind)
{
    Assert(QuadBatcher.TextureRegistryCount < MAX_TEXTURE_HANDLES);
    if(QuadBatcher.TextureRegistryCount >= MAX_TEXTURE_HANDLES)
    {
        return(0);
    }

    uint32 handle = QuadBatcher.TextureRegistryCount++;
    QuadBatcher.TextureRegistry[handle].Texture = texture;
    QuadBatcher.TextureRegistry[handle].Kind = kind;
    return(handle);
}

void D3D11QuadBatcherSubmitCommands(RenderCommands* commands)
{
    for(uint32 i = 0; i < commands->QuadCount; ++i)
    {
        RenderCommandQuad* quad = &commands->Quads[i];

        if(quad->TextureHandle == 0)
        {
            D3D11QuadBatcherPushQuad(quad->X, quad->Y, quad->Width, quad->Height, quad->U0, quad->V0, quad->U1, quad->V1, 0, quad->Color);
            continue;
        }

        TextureRegistryEntry* entry = &QuadBatcher.TextureRegistry[quad->TextureHandle];

        if(entry->Kind == GLYPH)
        {
            D3D11QuadBatcherPushGlyphQuad(quad->X, quad->Y, quad->Width, quad->Height, quad->U0, quad->V0, quad->U1, quad->V1, entry->Texture, quad->Color);
        }
        else
        {
            D3D11QuadBatcherPushQuad(quad->X, quad->Y, quad->Width, quad->Height, quad->U0, quad->V0, quad->U1, quad->V1, entry->Texture, quad->Color);
        }
    }
}

void D3D11QuadBatcherEnd()
{
    if(QuadBatcher.QuadCount == 0)
    {
        return;
    }

    D3D11_MAPPED_SUBRESOURCE mapped;
    if(FAILED(QuadBatcher.Context->Map(QuadBatcher.VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
    {
        return;
    }

    memcpy(mapped.pData, QuadBatcher.Vertices, QuadBatcher.QuadCount * 4 * sizeof(Vertex2D));
    QuadBatcher.Context->Unmap(QuadBatcher.VertexBuffer, 0);

    UINT stride = sizeof(Vertex2D);
    UINT offset = 0;

    QuadBatcher.Context->IASetInputLayout(QuadBatcher.InputLayout);
    QuadBatcher.Context->IASetVertexBuffers(0, 1, &QuadBatcher.VertexBuffer, &stride, &offset);
    QuadBatcher.Context->IASetIndexBuffer(QuadBatcher.IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    QuadBatcher.Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    QuadBatcher.Context->VSSetShader(QuadBatcher.VertexShader, 0, 0);
    QuadBatcher.Context->VSSetConstantBuffers(0, 1, &QuadBatcher.ConstantBuffer);

    QuadBatcher.Context->PSSetSamplers(0, 1, &QuadBatcher.Sampler);

    QuadBatcher.Context->RSSetState(QuadBatcher.RasterizerState);
    QuadBatcher.Context->OMSetBlendState(QuadBatcher.BlendState, 0, 0xFFFFFFFF);
    QuadBatcher.Context->OMSetDepthStencilState(QuadBatcher.DepthState, 0);

    UINT indexOffset = 0;
    for(uint32 batchIndex = 0; batchIndex < QuadBatcher.BatchCount; ++batchIndex)
    {
        TextureBatch* batch = &QuadBatcher.Batches[batchIndex];
        UINT indexCount = batch->QuadCount * 6;

        ID3D11PixelShader* pixelShader = (batch->Kind == GLYPH) ? QuadBatcher.PixelShaderAlpha : QuadBatcher.PixelShader;
        QuadBatcher.Context->PSSetShader(pixelShader, 0, 0);
        QuadBatcher.Context->PSSetShaderResources(0, 1, &batch->Texture);
        QuadBatcher.Context->DrawIndexed(indexCount, indexOffset, 0);

        indexOffset += indexCount;
    }
}
