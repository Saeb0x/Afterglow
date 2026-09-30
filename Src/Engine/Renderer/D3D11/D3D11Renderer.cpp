#include "Engine/Renderer/D3D11/D3D11Renderer.h"
#include "Engine/Renderer/D3D11/D3D11Gpu.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Gpu.h"
#include "Engine/Renderer/Internal/GpuInternal.h"

#include <SSTL/Core/Config.h>
#include <SSTL/Core/Utility.h>
#include <SSTL/Core/Assert.h>
#include <SSTL/Core/String.h>

#include <d3d11.h>
#include <d3d11_1.h>

#include <DirectXMath.h>

// NOTE(saeb): 16384 quads * 4 = 65536 vertices, the most a 16-bit index can address.
#define AG_MAX_QUADS 16384
SSTL_ASSERT_STATIC_MSG(AG_MAX_QUADS * 4 <= 65536, "Afterglow: Quad vertices must be addressable by 16-bit indices.");

#define AG_MAX_PIPELINES 64

// NOTE(saeb): Every quad belongs to a view, the matrix that maps its units to the screen. View 0 is the screen; the rest are this frame's cameras, in the order they were set.
#define AG_VIEW_SCREEN 0 // Screen pixels, y down
#define AG_MAX_VIEWS 16 // Cameras set in one frame, plus the screen; far more than parallax layers need

struct QuadVertex
{
    real32 X, Y;
    real32 U, V;
    real32 R, G, B, A;
};

struct QuadBatch
{
    RendererPipeline Pipeline;
    GpuTexture Texture;
    uint32 View;
    uint32 FirstQuad;
    uint32 QuadCount;
};

struct QuadPipeline
{
    ID3D11PixelShader* PixelShader;
};

struct QuadConstants
{
    DirectX::XMFLOAT4X4 ViewProjection; // Matches Quad.hlsl's ViewProjection: row-major, as DirectXMath stores it
};
SSTL_ASSERT_STATIC_MSG(sizeof(QuadConstants) % 16 == 0, "Afterglow: Constant buffers must be a multiple of 16 bytes.");

struct Renderer
{
    bool FrameActive;
    RendererSpace Space;
    Camera ViewCameras[AG_MAX_VIEWS]; // This frame's cameras by view; [0] is the screen, which needs none
    uint32 ViewCount;
    uint32 WorldView; // The view world quads are pushed under: the latest camera
    bool WorldViewUsed; // A world quad has been pushed under WorldView, so a new camera needs a new view
    GpuBuffer VertexBuffer;
    GpuBuffer IndexBuffer;
    RendererQuad* Quads;
    uint8* QuadViews; // Parallel to Quads: the view each one was pushed under
    uint32 QuadCount;
    uint32 DroppedQuadCount;
    RendererStats LastFrameStats;
    QuadBatch* Batches;
    ID3D11VertexShader* QuadVertexShader;
    ID3D11InputLayout* QuadInputLayout;
    GpuBuffer QuadConstantBuffer;
    ID3D11BlendState* BlendState;
    ID3D11RasterizerState* RasterizerState;
    ID3D11SamplerState* SamplerState;
    QuadPipeline Pipelines[AG_MAX_PIPELINES];
    uint32 PipelineCount;
    GpuTexture WhiteTexture;
    uint32 Flags;
};
static Renderer RendererData;

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

static uint32 D3D11FlushQuads()
{
    ID3D11DeviceContext* context = D3D11GpuGetContext();
    ID3DUserDefinedAnnotation* annotation = D3D11GpuGetAnnotation();

    QuadVertex* vertices = (QuadVertex*)GpuMapBuffer(RendererData.VertexBuffer);
    if(!vertices)
    {
        return(0);
    }

    int32 backBufferWidth, backBufferHeight;
    GpuGetBackBufferSize(&backBufferWidth, &backBufferHeight);

    // NOTE(saeb): Built at draw time from the final back buffer size. The screen maps pixels with y down (the larger y is "bottom"); each camera maps world metres with y up.
    DirectX::XMFLOAT4X4 views[AG_MAX_VIEWS];
    DirectX::XMStoreFloat4x4(&views[AG_VIEW_SCREEN], DirectX::XMMatrixOrthographicOffCenterLH(0.0f, (real32)backBufferWidth, (real32)backBufferHeight, 0.0f, 0.0f, 1.0f));
    for(uint32 view = 1; view < RendererData.ViewCount; ++view)
    {
        DirectX::XMStoreFloat4x4(&views[view], CameraGetViewProjection(&RendererData.ViewCameras[view], backBufferWidth, backBufferHeight));
    }

    QuadBatch* batch = nullptr;
    uint32 batchCount = 0;

    for(uint32 quadIndex = 0; quadIndex < RendererData.QuadCount; ++quadIndex)
    {
        const RendererQuad* quad = &RendererData.Quads[quadIndex];
        uint32 view = RendererData.QuadViews[quadIndex];

        // NOTE(saeb): Only consecutive quads merge; submission order is the layering order for alpha.
        if(!batch || batch->Pipeline != quad->Pipeline || batch->Texture.Object != quad->Texture.Object || batch->View != view)
        {
            batch = &RendererData.Batches[batchCount++];
            batch->Pipeline = quad->Pipeline;
            batch->Texture = quad->Texture;
            batch->View = view;
            batch->FirstQuad = quadIndex;
            batch->QuadCount = 0;
        }

        ++batch->QuadCount;

        // NOTE(saeb): Corners in vertex order: (X, Y), (X + W, Y), (X, Y + H), (X + W, Y + H).
        real32 x0 = quad->X;
        real32 y0 = quad->Y;
        real32 x1 = quad->X + quad->Width;
        real32 y1 = quad->Y + quad->Height;
        real32 cornerX[4] = { x0, x1, x0, x1 };
        real32 cornerY[4] = { y0, y0, y1, y1 };

        // NOTE(saeb): Turn each corner around the centre. Skipped at 0, so text and UI keep the exact axis-aligned path. Screen space has y down, which would turn the same formula clockwise, so its angle is negated: positive is counter-clockwise on screen in both spaces.
        if(quad->Rotation != 0.0f)
        {
            real32 rotation = (view == AG_VIEW_SCREEN) ? -quad->Rotation : quad->Rotation;
            real32 sine, cosine;
            DirectX::XMScalarSinCos(&sine, &cosine, rotation);

            real32 centreX = quad->X + quad->Width * 0.5f;
            real32 centreY = quad->Y + quad->Height * 0.5f;
            for(uint32 corner = 0; corner < 4; ++corner)
            {
                real32 offsetX = cornerX[corner] - centreX;
                real32 offsetY = cornerY[corner] - centreY;
                cornerX[corner] = centreX + offsetX * cosine - offsetY * sine;
                cornerY[corner] = centreY + offsetX * sine + offsetY * cosine;
            }
        }

        // NOTE(saeb): The game passes straight colors; premultiply here so the blend state's ONE is correct.
        real32 r = quad->R * quad->A;
        real32 g = quad->G * quad->A;
        real32 b = quad->B * quad->A;

        // NOTE(saeb): In world space y points up, so the quad's top edge is y1, not y0. Swapping V keeps the texture's top row at the top in both spaces.
        real32 v0 = quad->V0;
        real32 v1 = quad->V1;
        if(view != AG_VIEW_SCREEN)
        {
            v0 = quad->V1;
            v1 = quad->V0;
        }

        // NOTE(saeb): Mapped memory is write-combined; write each vertex whole, front to back, never read it back.
        QuadVertex* quadVertices = vertices + (quadIndex * 4);
        quadVertices[0] = { cornerX[0], cornerY[0], quad->U0, v0, r, g, b, quad->A }; // (X, Y): top-left on screen, bottom-left in the world
        quadVertices[1] = { cornerX[1], cornerY[1], quad->U1, v0, r, g, b, quad->A };
        quadVertices[2] = { cornerX[2], cornerY[2], quad->U0, v1, r, g, b, quad->A };
        quadVertices[3] = { cornerX[3], cornerY[3], quad->U1, v1, r, g, b, quad->A };
    }

    GpuUnmapBuffer(RendererData.VertexBuffer);

    ID3D11Buffer* vertexBuffer = D3D11GpuGetBuffer(RendererData.VertexBuffer);
    ID3D11Buffer* indexBuffer = D3D11GpuGetBuffer(RendererData.IndexBuffer);
    ID3D11Buffer* constantBuffer = D3D11GpuGetBuffer(RendererData.QuadConstantBuffer);
    ID3D11ShaderResourceView* whiteTexture = D3D11GpuGetTexture(RendererData.WhiteTexture);

    UINT stride = sizeof(QuadVertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    context->IASetInputLayout(RendererData.QuadInputLayout);
    context->VSSetShader(RendererData.QuadVertexShader, nullptr, 0);
    context->VSSetConstantBuffers(0, 1, &constantBuffer);
    context->RSSetState(RendererData.RasterizerState);
    context->PSSetSamplers(0, 1, &RendererData.SamplerState);
    context->OMSetBlendState(RendererData.BlendState, nullptr, 0xFFFFFFFF);

    RendererPipeline boundPipeline = UINT32_MAX;
    ID3D11ShaderResourceView* boundTexture = nullptr;
    uint32 boundView = UINT32_MAX;

    for(uint32 batchIndex = 0; batchIndex < batchCount; ++batchIndex)
    {
        QuadBatch* current = &RendererData.Batches[batchIndex];

        // NOTE(saeb): An invalid handle falls back to the defaults instead of binding garbage.
        RendererPipeline pipeline = (current->Pipeline < RendererData.PipelineCount) ? current->Pipeline : 0;
        ID3D11ShaderResourceView* texture = D3D11GpuGetTexture(current->Texture);
        if(!texture)
        {
            texture = whiteTexture;
        }

        if(pipeline != boundPipeline)
        {
            context->PSSetShader(RendererData.Pipelines[pipeline].PixelShader, nullptr, 0);
            boundPipeline = pipeline;
        }

        if(texture != boundTexture)
        {
            context->PSSetShaderResources(0, 1, &texture);
            boundTexture = texture;
        }

        if(current->View != boundView)
        {
            QuadConstants* constants = (QuadConstants*)GpuMapBuffer(RendererData.QuadConstantBuffer);
            if(constants)
            {
                constants->ViewProjection = views[current->View];
                GpuUnmapBuffer(RendererData.QuadConstantBuffer);
            }
            boundView = current->View;
        }

        // NOTE(saeb): One event per batch, so RenderDoc / PIX show why batches split. GetStatus() is TRUE only while a capture tool is attached, so normal runs never format the label.
        bool labeled = annotation && annotation->GetStatus();
        if(labeled)
        {
            wchar_t label[128];
            wsprintfW(label, L"Batch %u: %u quads, texture %08X, pipeline %u, view %u", batchIndex, current->QuadCount, (uint32)(usize)current->Texture.Object, pipeline, current->View);
            annotation->BeginEvent(label);
        }

        context->DrawIndexed(current->QuadCount * 6, current->FirstQuad * 6, 0);

        if(labeled)
        {
            annotation->EndEvent();
        }
    }

    return(batchCount);
}

bool D3D11RendererInit(StackAllocator* allocator)
{
    GpuBufferDesc vertexBufferDesc = {};
    vertexBufferDesc.Type = GpuBufferType::Vertex;
    vertexBufferDesc.Usage = GpuUsage::Dynamic;
    vertexBufferDesc.Size = AG_MAX_QUADS * 4 * sizeof(QuadVertex);
    vertexBufferDesc.DebugName = SV8(u8"QuadVertices");

    RendererData.VertexBuffer = GpuCreateBuffer(&vertexBufferDesc);
    if(!RendererData.VertexBuffer.Object)
    {
        return(false);
    }

    Frame frameScratch = GetFrame(allocator, Heap::Upper);

    uint32 indexCount = AG_MAX_QUADS * 6;
    uint16* indices = (uint16*)Allocate(allocator, Heap::Upper, indexCount * sizeof(uint16), alignof(uint16));
    if(!indices)
    {
        ReleaseFrame(allocator, frameScratch);
        return(false);
    }

    // NOTE(saeb): Vertex order per quad: 0 = top-left, 1 = top-right, 2 = bottom-left, 3 = bottom-right.
    for(uint32 quadIndex = 0; quadIndex < AG_MAX_QUADS; ++quadIndex)
    {
        uint16 firstVertex = (uint16)(quadIndex * 4);
        uint16* quadIndices = indices + (quadIndex * 6);

        quadIndices[0] = (uint16)(firstVertex + 0);
        quadIndices[1] = (uint16)(firstVertex + 1);
        quadIndices[2] = (uint16)(firstVertex + 2);
        quadIndices[3] = (uint16)(firstVertex + 2);
        quadIndices[4] = (uint16)(firstVertex + 1);
        quadIndices[5] = (uint16)(firstVertex + 3);
    }

    GpuBufferDesc indexBufferDesc = {};
    indexBufferDesc.Type = GpuBufferType::Index;
    indexBufferDesc.Usage = GpuUsage::Immutable;
    indexBufferDesc.Size = indexCount * sizeof(uint16);
    indexBufferDesc.Data = indices;
    indexBufferDesc.DebugName = SV8(u8"QuadIndices");

    RendererData.IndexBuffer = GpuCreateBuffer(&indexBufferDesc);

    ReleaseFrame(allocator, frameScratch);

    if(!RendererData.IndexBuffer.Object)
    {
        return(false);
    }

    // NOTE(saeb): AG_MAX_QUADS (16384) * 60 bytes = 960 KiB for the quads, 16384 bytes = 16 KiB for their views and 16384 * 20 bytes = 320 KiB for the batches. Sizing the batch array for the worst case (every quad changes state) means no check for running out of batches.
    RendererData.Quads = (RendererQuad*)Allocate(allocator, Heap::Lower, AG_MAX_QUADS * sizeof(RendererQuad), alignof(RendererQuad));
    RendererData.QuadViews = (uint8*)Allocate(allocator, Heap::Lower, AG_MAX_QUADS * sizeof(uint8), alignof(uint8));
    RendererData.Batches = (QuadBatch*)Allocate(allocator, Heap::Lower, AG_MAX_QUADS * sizeof(QuadBatch), alignof(QuadBatch));
    if(!RendererData.Quads || !RendererData.QuadViews || !RendererData.Batches)
    {
        return(false);
    }

    // NOTE(saeb): Slot 0 is reserved for the default pipeline, which RendererSetDefaultPipeline fills from the cooked shader; created pipelines start at 1.
    RendererData.PipelineCount = 1;

    // NOTE(saeb): View 1 starts as the default camera: a zeroed one is centred on the origin, one metre per pixel, zoom 1.
    RendererData.ViewCameras[1] = {};
    RendererData.ViewCount = 2;
    RendererData.WorldView = 1;
    RendererData.WorldViewUsed = false;

    GpuBufferDesc constantBufferDesc = {};
    constantBufferDesc.Type = GpuBufferType::Constant;
    constantBufferDesc.Usage = GpuUsage::Dynamic;
    constantBufferDesc.Size = sizeof(QuadConstants);
    constantBufferDesc.DebugName = SV8(u8"QuadConstants");

    RendererData.QuadConstantBuffer = GpuCreateBuffer(&constantBufferDesc);
    if(!RendererData.QuadConstantBuffer.Object)
    {
        return(false);
    }

    ID3D11Device* device = D3D11GpuGetDevice();

    // NOTE(saeb): Premultiplied "over": color = src + dst * (1 - srcAlpha); src.rgb already carries its alpha.
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    if(FAILED(device->CreateBlendState(&blendDesc, &RendererData.BlendState)))
    {
        return(false);
    }

    D3D11SetName(RendererData.BlendState, SV8(u8"PremultipliedBlend"));

    D3D11_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_NONE; // A negative width/height flips winding; still draw it
    rasterizerDesc.DepthClipEnable = TRUE; // D3D11's default is TRUE, but a zeroed desc makes it FALSE

    if(FAILED(device->CreateRasterizerState(&rasterizerDesc, &RendererData.RasterizerState)))
    {
        return(false);
    }

    D3D11SetName(RendererData.RasterizerState, SV8(u8"CullNoneRasterizer"));

    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // Art is scaled to the window, so blend texels; point sampling makes scaled edges jagged
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX; // Zeroed would lock every texture to mip 0

    if(FAILED(device->CreateSamplerState(&samplerDesc, &RendererData.SamplerState)))
    {
        return(false);
    }

    D3D11SetName(RendererData.SamplerState, SV8(u8"PointClampSampler"));

    // NOTE(saeb): Quads with no texture ({0}) or a destroyed one sample this, so they draw solid.
    uint32 whitePixel = 0xFFFFFFFF;
    GpuTextureDesc whiteDesc = {};
    whiteDesc.Width = 1;
    whiteDesc.Height = 1;
    whiteDesc.Format = GpuFormat::RGBA8;
    whiteDesc.Data = &whitePixel;
    whiteDesc.DebugName = SV8(u8"WhiteTexture");

    RendererData.WhiteTexture = GpuCreateTexture(&whiteDesc);
    if(!RendererData.WhiteTexture.Object)
    {
        return(false);
    }

    return(true);
}

void D3D11RendererBeginFrame(int32 width, int32 height)
{
    RendererData.FrameActive = GpuBeginFrame(width, height);
    if(RendererData.FrameActive)
    {
        GpuPassDesc pass = { { 0.529f, 0.808f, 0.922f, 1.0f } };
        GpuBeginPass(&pass);
    }
}

void D3D11RendererEndFrame()
{
    // NOTE(saeb): The input layout is the last object RendererSetDefaultPipeline creates; without it there's nothing to draw quads with.
    uint32 drawCalls = 0;
    if(RendererData.FrameActive && RendererData.QuadInputLayout && RendererData.QuadCount > 0)
    {
        GpuBeginMarker(SV8(u8"Quads"));
        drawCalls = D3D11FlushQuads();
        GpuEndMarker();
    }

    if(RendererData.FrameActive)
    {
        GpuEndPass();
    }

    RendererData.LastFrameStats.Quads = RendererData.QuadCount;
    RendererData.LastFrameStats.DrawCalls = drawCalls;
    RendererData.LastFrameStats.DroppedQuads = RendererData.DroppedQuadCount;

    // NOTE(saeb): Reset here, not in BeginFrame; BeginFrame can early-out and would leave stale quads behind.
    RendererData.QuadCount = 0;
    RendererData.DroppedQuadCount = 0;
    RendererData.Space = RendererSpace::World;

    // NOTE(saeb): The camera in use at the end of the frame carries over as the next frame's view 1, so a camera set once keeps applying.
    RendererData.ViewCameras[1] = RendererData.ViewCameras[RendererData.WorldView];
    RendererData.ViewCount = 2;
    RendererData.WorldView = 1;
    RendererData.WorldViewUsed = false;

    GpuPresent((RendererData.Flags & RendererFlags_VSync) != 0);
}

void D3D11RendererShutdown()
{
    GpuDestroyTexture(RendererData.WhiteTexture);
    RendererData.WhiteTexture = {};

    // NOTE(saeb): Walk the full array, not just up to the count; a failed Init can create an object before its count is set.
    for(uint32 pipelineIndex = 0; pipelineIndex < AG_MAX_PIPELINES; ++pipelineIndex)
    {
        if(RendererData.Pipelines[pipelineIndex].PixelShader)
        {
            RendererData.Pipelines[pipelineIndex].PixelShader->Release();
            RendererData.Pipelines[pipelineIndex].PixelShader = nullptr;
        }
    }
    RendererData.PipelineCount = 0;

    if(RendererData.SamplerState)
    {
        RendererData.SamplerState->Release();
        RendererData.SamplerState = nullptr;
    }

    if(RendererData.RasterizerState)
    {
        RendererData.RasterizerState->Release();
        RendererData.RasterizerState = nullptr;
    }

    if(RendererData.BlendState)
    {
        RendererData.BlendState->Release();
        RendererData.BlendState = nullptr;
    }

    if(RendererData.QuadInputLayout)
    {
        RendererData.QuadInputLayout->Release();
        RendererData.QuadInputLayout = nullptr;
    }

    if(RendererData.QuadVertexShader)
    {
        RendererData.QuadVertexShader->Release();
        RendererData.QuadVertexShader = nullptr;
    }

    GpuDestroyBuffer(RendererData.QuadConstantBuffer);
    GpuDestroyBuffer(RendererData.IndexBuffer);
    GpuDestroyBuffer(RendererData.VertexBuffer);
    RendererData.QuadConstantBuffer = {};
    RendererData.IndexBuffer = {};
    RendererData.VertexBuffer = {};

    // NOTE(saeb): The quad, view and batch arrays live in the engine's Lower heap; ShutdownStackAllocator frees them.
    RendererData.Quads = nullptr;
    RendererData.QuadViews = nullptr;
    RendererData.Batches = nullptr;
    RendererData.QuadCount = 0;
}

void RendererSetFlags(uint32 rendererFlags)
{
    RendererData.Flags = rendererFlags;
}

void RendererGetStats(RendererStats* stats)
{
    *stats = RendererData.LastFrameStats;
    stats->Pipelines = RendererData.PipelineCount;
    stats->MaxPipelines = AG_MAX_PIPELINES;
    stats->MaxQuads = AG_MAX_QUADS;
}

void RendererSetSpace(RendererSpace space)
{
    RendererData.Space = space;
}

RendererSpace RendererGetSpace()
{
    return(RendererData.Space);
}

void RendererPushQuad(const RendererQuad* quad)
{
    // NOTE(saeb): Full; drop the quad rather than overflow. The vertex buffer can't hold more anyway. Counted, so the stats show it.
    if(RendererData.QuadCount >= AG_MAX_QUADS)
    {
        ++RendererData.DroppedQuadCount;
        return;
    }

    // NOTE(saeb): Stored as given, in its own units; the view's matrix maps it to the screen at draw time.
    uint32 index = RendererData.QuadCount++;
    RendererData.Quads[index] = *quad;
    if(RendererData.Space == RendererSpace::Screen)
    {
        RendererData.QuadViews[index] = AG_VIEW_SCREEN;
    }
    else
    {
        RendererData.QuadViews[index] = (uint8)RendererData.WorldView;
        RendererData.WorldViewUsed = true;
    }
}

RendererPipeline RendererCreatePipeline(const uint8* pixelBytecode, usize size, StringView8 debugName)
{
    // NOTE(saeb): Not initialized, table full, or creation failed: return the default pipeline, so the quad still draws instead of crashing.
    ID3D11Device* device = D3D11GpuGetDevice();
    if(!device || !D3D11IsBytecodeValid(pixelBytecode, size) || RendererData.PipelineCount >= AG_MAX_PIPELINES)
    {
        return(0);
    }

    // NOTE(saeb): D3D11 checks the bytecode's own checksum here; with the debug layer set to break on errors, damaged bytecode stops in the debugger.
    ID3D11PixelShader** pixelShader = &RendererData.Pipelines[RendererData.PipelineCount].PixelShader;
    if(FAILED(device->CreatePixelShader(pixelBytecode, size, nullptr, pixelShader)))
    {
        *pixelShader = nullptr;
        return(0);
    }

    D3D11SetName(*pixelShader, debugName);

    return(RendererData.PipelineCount++);
}

bool RendererSetDefaultPipeline(const uint8* vertexBytecode, usize vertexSize, const uint8* pixelBytecode, usize pixelSize)
{
    // NOTE(saeb): Once only; replacing shaders at runtime (hot reload) would also need to release the old ones.
    ID3D11Device* device = D3D11GpuGetDevice();
    if(!device || RendererData.QuadVertexShader || !D3D11IsBytecodeValid(vertexBytecode, vertexSize) || !D3D11IsBytecodeValid(pixelBytecode, pixelSize))
    {
        return(false);
    }

    // NOTE(saeb): The input layout is validated against the vertex shader's input signature, so it needs the VS bytecode.
    D3D11_INPUT_ELEMENT_DESC inputElements[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(QuadVertex, X), D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(QuadVertex, U), D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(QuadVertex, R), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

    bool created = SUCCEEDED(device->CreateVertexShader(vertexBytecode, vertexSize, nullptr, &RendererData.QuadVertexShader)) &&
        SUCCEEDED(device->CreatePixelShader(pixelBytecode, pixelSize, nullptr, &RendererData.Pipelines[0].PixelShader)) &&
        SUCCEEDED(device->CreateInputLayout(inputElements, SSTL_ARRAYCOUNT(inputElements), vertexBytecode, vertexSize, &RendererData.QuadInputLayout));

    if(created)
    {
        D3D11SetName(RendererData.QuadVertexShader, SV8(u8"QuadVS"));
        D3D11SetName(RendererData.Pipelines[0].PixelShader, SV8(u8"QuadPS"));
        D3D11SetName(RendererData.QuadInputLayout, SV8(u8"QuadInputLayout"));
    }

    if(!created)
    {
        // NOTE(saeb): All or nothing; EndFrame treats the input layout as "ready", so never leave half a set behind.
        if(RendererData.QuadVertexShader)
        {
            RendererData.QuadVertexShader->Release();
            RendererData.QuadVertexShader = nullptr;
        }

        if(RendererData.Pipelines[0].PixelShader)
        {
            RendererData.Pipelines[0].PixelShader->Release();
            RendererData.Pipelines[0].PixelShader = nullptr;
        }

        if(RendererData.QuadInputLayout)
        {
            RendererData.QuadInputLayout->Release();
            RendererData.QuadInputLayout = nullptr;
        }

        return(false);
    }

    return(true);
}

void RendererSetCamera(const Camera* camera)
{
    // NOTE(saeb): Nothing has been drawn with the current camera yet, so replace it rather than add a view: a camera set every frame keeps reusing view 1, and setting one several times before drawing can't fill the table.
    if(!RendererData.WorldViewUsed)
    {
        RendererData.ViewCameras[RendererData.WorldView] = *camera;
        return;
    }

    // NOTE(saeb): Full: keep drawing with the last camera rather than overflow. Sixteen cameras in one frame means something is setting one per object.
    if(RendererData.ViewCount >= AG_MAX_VIEWS)
    {
        return;
    }

    RendererData.ViewCameras[RendererData.ViewCount] = *camera;
    RendererData.WorldView = RendererData.ViewCount++;
    RendererData.WorldViewUsed = false;
}
