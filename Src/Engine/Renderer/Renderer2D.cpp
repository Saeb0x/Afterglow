#include "Engine/Renderer/Renderer2D.h"
#include "Engine/Renderer/Internal/Renderer2DInternal.h"
#include "Engine/Renderer/Gpu.h"
#include "Engine/Renderer/Internal/GpuInternal.h"
#include "Engine/Asset/Internal/AssetInternal.h"

#include <SSTL/Core/Config.h>
#include <SSTL/Core/Utility.h>
#include <SSTL/Core/Assert.h>
#include <SSTL/Core/String.h>

#include <DirectXMath.h>

// NOTE(saeb): 16384 quads * 4 = 65536 vertices, the most a 16-bit index can address.
#define AG_MAX_QUADS 16384
SSTL_ASSERT_STATIC_MSG(AG_MAX_QUADS * 4 <= 65536, "Afterglow: Quad vertices must be addressable by 16-bit indices.");

#define AG_MAX_PIPELINES 64

#define AG_QUAD_SHADER_PATH u8"Data/Engine/Quad.aga"

// NOTE(saeb): Every quad belongs to a view, the matrix that maps its units to the screen. View 0 is the screen; the rest are this frame's cameras, in the order they were set.
#define AG_VIEW_SCREEN 0 // Screen pixels, y down
#define AG_MAX_VIEWS 16 // Cameras set in one frame, plus the screen; far more than parallax layers need

// NOTE(saeb): What the pixel shader does with a vertex; Quad.hlsl reads the same values.
#define AG_QUAD_SOLID 0.0f
#define AG_QUAD_TEXTURED 1.0f
#define AG_QUAD_TEXT 2.0f

struct QuadVertex
{
    real32 X, Y;
    real32 U, V;
    real32 R, G, B, A;
    real32 Mode; // AG_QUAD_SOLID, AG_QUAD_TEXTURED or AG_QUAD_TEXT
};

struct QuadBatch
{
    Renderer2DPipeline Pipeline;
    GpuTexture Texture;
    uint32 View;
    uint32 FirstQuad;
    uint32 QuadCount;
};

struct QuadConstants
{
    DirectX::XMFLOAT4X4 ViewProjection; // Matches Quad.hlsl's ViewProjection: row-major, as DirectXMath stores it
};
SSTL_ASSERT_STATIC_MSG(sizeof(QuadConstants) % 16 == 0, "Afterglow: Constant buffers must be a multiple of 16 bytes.");

struct Renderer2D
{
    Renderer2DSpace Space;
    Camera ViewCameras[AG_MAX_VIEWS]; // This frame's cameras by view; [0] is the screen, which needs none
    uint32 ViewCount;
    uint32 WorldView; // The view world quads are pushed under: the latest camera
    bool WorldViewUsed; // A world quad has been pushed under WorldView, so a new camera needs a new view
    GpuBuffer VertexBuffer;
    GpuBuffer IndexBuffer;
    Renderer2DQuad* Quads;
    uint8* QuadViews; // Parallel to Quads: the view each one was pushed under
    uint32 QuadCount;
    uint32 DroppedQuadCount;
    Renderer2DStats LastFrameStats;
    QuadBatch* Batches;
    GpuBuffer QuadConstantBuffer;
    const uint8* QuadVertexShader; // A copy in the Lower heap, for pipelines that don't bring their own
    usize QuadVertexShaderSize;
    GpuPipeline Pipelines[AG_MAX_PIPELINES];
    uint32 PipelineCount;
    GpuTexture WhiteTexture;
};
static Renderer2D Renderer2DData;

static GpuPipeline Renderer2DCreateQuadPipeline(StackAllocator* allocator, const Renderer2DPipelineDesc* quadDesc)
{
    GpuPipelineDesc desc = {};
    desc.VertexShader = quadDesc->VertexShader ? quadDesc->VertexShader : Renderer2DData.QuadVertexShader;
    desc.VertexShaderSize = quadDesc->VertexShader ? quadDesc->VertexShaderSize : Renderer2DData.QuadVertexShaderSize;
    desc.PixelShader = quadDesc->PixelShader;
    desc.PixelShaderSize = quadDesc->PixelShaderSize;
    desc.Attributes[0] = { 0, GpuVertexFormat::Float2, offsetof(QuadVertex, X) };
    desc.Attributes[1] = { 1, GpuVertexFormat::Float2, offsetof(QuadVertex, U) };
    desc.Attributes[2] = { 2, GpuVertexFormat::Float4, offsetof(QuadVertex, R) };
    desc.Attributes[3] = { 3, GpuVertexFormat::Float, offsetof(QuadVertex, Mode) };
    desc.AttributeCount = 4;
    desc.Blend = (quadDesc->Blend == Renderer2DBlend::Additive) ? GpuBlend::Additive : GpuBlend::Premultiplied;
    desc.Cull = GpuCull::None; // A negative width/height flips winding; still draw it
    desc.Primitive = GpuPrimitive::Triangles;
    desc.DebugName = quadDesc->DebugName;

    return(GpuCreatePipeline(allocator, &desc));
}

static uint32 Renderer2DFlushQuads()
{
    QuadVertex* vertices = (QuadVertex*)GpuMapBuffer(Renderer2DData.VertexBuffer);
    if(!vertices)
    {
        return(0);
    }

    int32 backBufferWidth, backBufferHeight;
    GpuGetBackBufferSize(&backBufferWidth, &backBufferHeight);

    // NOTE(saeb): Built at draw time from the final back buffer size. The screen maps pixels with y down (the larger y is "bottom"); each camera maps world metres with y up.
    DirectX::XMFLOAT4X4 views[AG_MAX_VIEWS];
    DirectX::XMStoreFloat4x4(&views[AG_VIEW_SCREEN], DirectX::XMMatrixOrthographicOffCenterLH(0.0f, (real32)backBufferWidth, (real32)backBufferHeight, 0.0f, 0.0f, 1.0f));
    for(uint32 view = 1; view < Renderer2DData.ViewCount; ++view)
    {
        DirectX::XMStoreFloat4x4(&views[view], CameraGetViewProjection(&Renderer2DData.ViewCameras[view], backBufferWidth, backBufferHeight));
    }

    QuadBatch* batch = nullptr;
    uint32 batchCount = 0;

    for(uint32 quadIndex = 0; quadIndex < Renderer2DData.QuadCount; ++quadIndex)
    {
        const Renderer2DQuad* quad = &Renderer2DData.Quads[quadIndex];
        uint32 view = Renderer2DData.QuadViews[quadIndex];

        // NOTE(saeb): Only consecutive quads merge; submission order is the layering order for alpha. A solid quad never samples its texture, so it joins a batch with any texture, and a batch of only solid quads takes the texture of the first textured quad that joins it.
        bool textured = (quad->Texture.Object != nullptr);
        bool textureConflict = textured && batch && batch->Texture.Object && batch->Texture.Object != quad->Texture.Object;
        if(!batch || batch->Pipeline != quad->Pipeline || textureConflict || batch->View != view)
        {
            batch = &Renderer2DData.Batches[batchCount++];
            batch->Pipeline = quad->Pipeline;
            batch->Texture = {};
            batch->View = view;
            batch->FirstQuad = quadIndex;
            batch->QuadCount = 0;
        }

        if(textured)
        {
            batch->Texture = quad->Texture;
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

        real32 mode = AG_QUAD_SOLID;
        if(textured)
        {
            mode = (quad->Mode == Renderer2DMode::Text) ? AG_QUAD_TEXT : AG_QUAD_TEXTURED;
        }

        // NOTE(saeb): Mapped memory is write-combined; write each vertex whole, front to back, never read it back.
        QuadVertex* quadVertices = vertices + (quadIndex * 4);
        quadVertices[0] = { cornerX[0], cornerY[0], quad->U0, v0, r, g, b, quad->A, mode }; // (X, Y): top-left on screen, bottom-left in the world
        quadVertices[1] = { cornerX[1], cornerY[1], quad->U1, v0, r, g, b, quad->A, mode };
        quadVertices[2] = { cornerX[2], cornerY[2], quad->U0, v1, r, g, b, quad->A, mode };
        quadVertices[3] = { cornerX[3], cornerY[3], quad->U1, v1, r, g, b, quad->A, mode };
    }

    GpuUnmapBuffer(Renderer2DData.VertexBuffer);

    GpuSetVertexBuffer(Renderer2DData.VertexBuffer, sizeof(QuadVertex));
    GpuSetIndexBuffer(Renderer2DData.IndexBuffer, GpuIndexFormat::U16);
    GpuSetConstantBuffer(0, Renderer2DData.QuadConstantBuffer);

    Renderer2DPipeline boundPipeline = UINT32_MAX;
    void* boundTexture = nullptr;
    uint32 boundView = UINT32_MAX;
    bool labeled = GpuMarkersEnabled();

    for(uint32 batchIndex = 0; batchIndex < batchCount; ++batchIndex)
    {
        QuadBatch* current = &Renderer2DData.Batches[batchIndex];

        // NOTE(saeb): An invalid handle falls back to the defaults instead of binding garbage.
        Renderer2DPipeline pipeline = (current->Pipeline < Renderer2DData.PipelineCount && Renderer2DData.Pipelines[current->Pipeline].Object) ? current->Pipeline : 0;
        // NOTE(saeb): A batch of only solid quads samples nothing, but the shader still needs a texture bound.
        GpuTexture texture = current->Texture.Object ? current->Texture : Renderer2DData.WhiteTexture;

        if(pipeline != boundPipeline)
        {
            GpuSetPipeline(Renderer2DData.Pipelines[pipeline]);
            boundPipeline = pipeline;
        }

        if(texture.Object != boundTexture)
        {
            GpuSetTexture(0, texture, GpuSampler::LinearClamp); // Art is scaled to the window, so blend texels; point sampling makes scaled edges jagged
            boundTexture = texture.Object;
        }

        if(current->View != boundView)
        {
            QuadConstants* constants = (QuadConstants*)GpuMapBuffer(Renderer2DData.QuadConstantBuffer);
            if(constants)
            {
                constants->ViewProjection = views[current->View];
                GpuUnmapBuffer(Renderer2DData.QuadConstantBuffer);
            }
            boundView = current->View;
        }

        // NOTE(saeb): One event per batch, so RenderDoc / PIX show why batches split. Labels are only formatted while a capture tool is attached.
        if(labeled)
        {
            char8 buffer[128];
            String8 label = { buffer, 0, sizeof(buffer) };
            String8Append(&label, SV8(u8"Batch "));
            String8AppendUInt(&label, batchIndex);
            String8Append(&label, SV8(u8": "));
            String8AppendUInt(&label, current->QuadCount);
            String8Append(&label, SV8(u8" quads, pipeline "));
            String8AppendUInt(&label, pipeline);
            String8Append(&label, SV8(u8", view "));
            String8AppendUInt(&label, current->View);
            GpuBeginMarker(StringView8{ label.Data, label.Length });
        }

        GpuDrawIndexed(current->QuadCount * 6, current->FirstQuad * 6);

        if(labeled)
        {
            GpuEndMarker();
        }
    }

    return(batchCount);
}

static bool Renderer2DCreateDefaultPipeline(StackAllocator* allocator, const uint8* vertexBytecode, usize vertexSize, const uint8* pixelBytecode, usize pixelSize)
{
    // NOTE(saeb): All or nothing: on failure the copy is given back.
    Frame lowerFrame = GetFrame(allocator, Heap::Lower);

    uint8* vertexCopy = (uint8*)Allocate(allocator, Heap::Lower, vertexSize, 16);
    if(!vertexCopy)
    {
        return(false);
    }

    for(usize index = 0; index < vertexSize; ++index)
    {
        vertexCopy[index] = vertexBytecode[index];
    }

    Renderer2DData.QuadVertexShader = vertexCopy;
    Renderer2DData.QuadVertexShaderSize = vertexSize;

    Renderer2DPipelineDesc desc = {};
    desc.PixelShader = pixelBytecode;
    desc.PixelShaderSize = pixelSize;
    desc.DebugName = SV8(u8"QuadPipeline");

    GpuPipeline pipeline = Renderer2DCreateQuadPipeline(allocator, &desc);
    if(!pipeline.Object)
    {
        Renderer2DData.QuadVertexShader = nullptr;
        Renderer2DData.QuadVertexShaderSize = 0;
        ReleaseFrame(allocator, lowerFrame);
        return(false);
    }

    Renderer2DData.Pipelines[0] = pipeline;

    return(true);
}

bool Renderer2DInit(StackAllocator* allocator, RendererInitError* error)
{
    *error = { SV8(u8"Couldn't create the 2D renderer's GPU resources."), StringView8{ nullptr, 0 } };

    GpuBufferDesc vertexBufferDesc = {};
    vertexBufferDesc.Type = GpuBufferType::Vertex;
    vertexBufferDesc.Usage = GpuUsage::Dynamic;
    vertexBufferDesc.Size = AG_MAX_QUADS * 4 * sizeof(QuadVertex);
    vertexBufferDesc.DebugName = SV8(u8"QuadVertices");

    Renderer2DData.VertexBuffer = GpuCreateBuffer(&vertexBufferDesc);
    if(!Renderer2DData.VertexBuffer.Object)
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

    Renderer2DData.IndexBuffer = GpuCreateBuffer(&indexBufferDesc);

    ReleaseFrame(allocator, frameScratch);

    if(!Renderer2DData.IndexBuffer.Object)
    {
        return(false);
    }

    // NOTE(saeb): AG_MAX_QUADS (16384) * 60 bytes = 960 KiB for the quads, 16384 bytes = 16 KiB for their views and 16384 * 20 bytes = 320 KiB for the batches. Sizing the batch array for the worst case (every quad changes state) means no check for running out of batches.
    Renderer2DData.Quads = (Renderer2DQuad*)Allocate(allocator, Heap::Lower, AG_MAX_QUADS * sizeof(Renderer2DQuad), alignof(Renderer2DQuad));
    Renderer2DData.QuadViews = (uint8*)Allocate(allocator, Heap::Lower, AG_MAX_QUADS * sizeof(uint8), alignof(uint8));
    Renderer2DData.Batches = (QuadBatch*)Allocate(allocator, Heap::Lower, AG_MAX_QUADS * sizeof(QuadBatch), alignof(QuadBatch));
    if(!Renderer2DData.Quads || !Renderer2DData.QuadViews || !Renderer2DData.Batches)
    {
        return(false);
    }

    // NOTE(saeb): Slot 0 is reserved for the default pipeline, which Init fills from the quad shader; created pipelines start at 1.
    Renderer2DData.PipelineCount = 1;

    // NOTE(saeb): View 1 starts as the default camera: a zeroed one is centred on the origin, one metre per pixel, zoom 1.
    Renderer2DData.ViewCameras[1] = {};
    Renderer2DData.ViewCount = 2;
    Renderer2DData.WorldView = 1;
    Renderer2DData.WorldViewUsed = false;

    GpuBufferDesc constantBufferDesc = {};
    constantBufferDesc.Type = GpuBufferType::Constant;
    constantBufferDesc.Usage = GpuUsage::Dynamic;
    constantBufferDesc.Size = sizeof(QuadConstants);
    constantBufferDesc.DebugName = SV8(u8"QuadConstants");

    Renderer2DData.QuadConstantBuffer = GpuCreateBuffer(&constantBufferDesc);
    if(!Renderer2DData.QuadConstantBuffer.Object)
    {
        return(false);
    }

    // NOTE(saeb): Quads with no texture ({0}) or a destroyed one sample this, so they draw solid.
    uint32 whitePixel = 0xFFFFFFFF;
    GpuTextureDesc whiteDesc = {};
    whiteDesc.Width = 1;
    whiteDesc.Height = 1;
    whiteDesc.Format = GpuFormat::RGBA8;
    whiteDesc.Data = &whitePixel;
    whiteDesc.DebugName = SV8(u8"WhiteTexture");

    Renderer2DData.WhiteTexture = GpuCreateTexture(&whiteDesc);
    if(!Renderer2DData.WhiteTexture.Object)
    {
        return(false);
    }

    // NOTE(saeb): The quad shader every pipeline 0 quad draws with; the driver keeps its own copy of the bytecode, so the file is scratch.
    Frame scratch = GetFrame(allocator, Heap::Upper);

    AssetShader shader;
    AssetLoadResult shaderResult = AssetReadShader(allocator, SV8(AG_QUAD_SHADER_PATH), &shader);
    if(shaderResult == AssetLoadResult::Ok && (!shader.Vertex || !shader.Pixel))
    {
        shaderResult = AssetLoadResult::MissingStage;
    }

    bool created = (shaderResult == AssetLoadResult::Ok) && Renderer2DCreateDefaultPipeline(allocator, shader.Vertex, shader.VertexSize, shader.Pixel, shader.PixelSize);

    ReleaseFrame(allocator, scratch);

    if(!created)
    {
        error->Message = SV8(u8"Couldn't load the quad shader (" AG_QUAD_SHADER_PATH u8").");
        error->Reason = (shaderResult != AssetLoadResult::Ok) ? AssetDescribeResult(shaderResult) : AssetDescribeResult(AssetLoadResult::RendererFailed);
        return(false);
    }

    return(true);
}

void Renderer2DEndFrame(bool draw)
{
    uint32 drawCalls = 0;
    if(draw && Renderer2DData.QuadCount > 0)
    {
        GpuBeginMarker(SV8(u8"Quads"));
        drawCalls = Renderer2DFlushQuads();
        GpuEndMarker();
    }

    Renderer2DData.LastFrameStats.Quads = Renderer2DData.QuadCount;
    Renderer2DData.LastFrameStats.DrawCalls = drawCalls;
    Renderer2DData.LastFrameStats.DroppedQuads = Renderer2DData.DroppedQuadCount;

    // NOTE(saeb): Reset here, not in BeginFrame; BeginFrame can early-out and would leave stale quads behind.
    Renderer2DData.QuadCount = 0;
    Renderer2DData.DroppedQuadCount = 0;
    Renderer2DData.Space = Renderer2DSpace::World;

    // NOTE(saeb): The camera in use at the end of the frame carries over as the next frame's view 1, so a camera set once keeps applying.
    Renderer2DData.ViewCameras[1] = Renderer2DData.ViewCameras[Renderer2DData.WorldView];
    Renderer2DData.ViewCount = 2;
    Renderer2DData.WorldView = 1;
    Renderer2DData.WorldViewUsed = false;
}

void Renderer2DShutdown()
{
    GpuDestroyTexture(Renderer2DData.WhiteTexture);
    Renderer2DData.WhiteTexture = {};

    // NOTE(saeb): Walk the full array, not just up to the count; slot 0 is filled after Init.
    for(uint32 pipelineIndex = 0; pipelineIndex < AG_MAX_PIPELINES; ++pipelineIndex)
    {
        GpuDestroyPipeline(Renderer2DData.Pipelines[pipelineIndex]);
        Renderer2DData.Pipelines[pipelineIndex] = {};
    }
    Renderer2DData.PipelineCount = 0;

    // NOTE(saeb): The copy lives in the engine's Lower heap, like the pipeline records.
    Renderer2DData.QuadVertexShader = nullptr;
    Renderer2DData.QuadVertexShaderSize = 0;

    GpuDestroyBuffer(Renderer2DData.QuadConstantBuffer);
    GpuDestroyBuffer(Renderer2DData.IndexBuffer);
    GpuDestroyBuffer(Renderer2DData.VertexBuffer);
    Renderer2DData.QuadConstantBuffer = {};
    Renderer2DData.IndexBuffer = {};
    Renderer2DData.VertexBuffer = {};

    // NOTE(saeb): The quad, view and batch arrays live in the engine's Lower heap; ShutdownStackAllocator frees them.
    Renderer2DData.Quads = nullptr;
    Renderer2DData.QuadViews = nullptr;
    Renderer2DData.Batches = nullptr;
    Renderer2DData.QuadCount = 0;
}

void Renderer2DGetStats(Renderer2DStats* stats)
{
    *stats = Renderer2DData.LastFrameStats;
    stats->Pipelines = Renderer2DData.PipelineCount;
    stats->MaxPipelines = AG_MAX_PIPELINES;
    stats->MaxQuads = AG_MAX_QUADS;
}

void Renderer2DSetSpace(Renderer2DSpace space)
{
    Renderer2DData.Space = space;
}

Renderer2DSpace Renderer2DGetSpace()
{
    return(Renderer2DData.Space);
}

void Renderer2DPushQuad(const Renderer2DQuad* quad)
{
    // NOTE(saeb): Full; drop the quad rather than overflow. The vertex buffer can't hold more anyway. Counted, so the stats show it.
    if(Renderer2DData.QuadCount >= AG_MAX_QUADS)
    {
        ++Renderer2DData.DroppedQuadCount;
        return;
    }

    // NOTE(saeb): Stored as given, in its own units; the view's matrix maps it to the screen at draw time.
    uint32 index = Renderer2DData.QuadCount++;
    Renderer2DData.Quads[index] = *quad;
    if(Renderer2DData.Space == Renderer2DSpace::Screen)
    {
        Renderer2DData.QuadViews[index] = AG_VIEW_SCREEN;
    }
    else
    {
        Renderer2DData.QuadViews[index] = (uint8)Renderer2DData.WorldView;
        Renderer2DData.WorldViewUsed = true;
    }
}

Renderer2DPipeline Renderer2DCreatePipeline(StackAllocator* allocator, const Renderer2DPipelineDesc* desc)
{
    // NOTE(saeb): Table full or creation failed: return the default pipeline, so the quad still draws instead of crashing.
    if(Renderer2DData.PipelineCount >= AG_MAX_PIPELINES)
    {
        return(0);
    }

    GpuPipeline pipeline = Renderer2DCreateQuadPipeline(allocator, desc);
    if(!pipeline.Object)
    {
        return(0);
    }

    Renderer2DData.Pipelines[Renderer2DData.PipelineCount] = pipeline;

    return(Renderer2DData.PipelineCount++);
}

void Renderer2DSetCamera(const Camera* camera)
{
    // NOTE(saeb): Nothing has been drawn with the current camera yet, so replace it rather than add a view: a camera set every frame keeps reusing view 1, and setting one several times before drawing can't fill the table.
    if(!Renderer2DData.WorldViewUsed)
    {
        Renderer2DData.ViewCameras[Renderer2DData.WorldView] = *camera;
        return;
    }

    // NOTE(saeb): Full: keep drawing with the last camera rather than overflow. Sixteen cameras in one frame means something is setting one per object.
    if(Renderer2DData.ViewCount >= AG_MAX_VIEWS)
    {
        return;
    }

    Renderer2DData.ViewCameras[Renderer2DData.ViewCount] = *camera;
    Renderer2DData.WorldView = Renderer2DData.ViewCount++;
    Renderer2DData.WorldViewUsed = false;
}
