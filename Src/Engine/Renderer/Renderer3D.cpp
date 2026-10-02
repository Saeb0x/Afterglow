#include "Engine/Renderer/Renderer3D.h"
#include "Engine/Renderer/Internal/Renderer3DInternal.h"
#include "Engine/Renderer/Internal/GpuInternal.h"
#include "Engine/Asset/Internal/AssetInternal.h"

#include <SSTL/Core/Assert.h>

#include <stddef.h>

#define AG_MESH_SHADER_PATH u8"Data/Engine/Mesh.aga"
#define AG_MAX_MESH_DRAWS 4096

// NOTE(saeb): Everything one mesh draw needs; matches Mesh.hlsl's MeshConstants. Matrices are row-major, as DirectXMath stores them.
struct MeshConstants
{
    DirectX::XMFLOAT4X4 ViewProjection;
    DirectX::XMFLOAT4X4 World;
    real32 Color[4];
    real32 ToSun[4]; // xyz: the unit direction from any surface toward the sun
    real32 SunColor[4];
    real32 AmbientColor[4];
};
SSTL_ASSERT_STATIC_MSG(sizeof(MeshConstants) % 16 == 0, "Afterglow: Constant buffers must be a multiple of 16 bytes.");

struct MeshDraw
{
    Renderer3DMesh Mesh;
    DirectX::XMFLOAT4X4 World;
    real32 R, G, B;
};

struct Renderer3D
{
    GpuPipeline Pipeline;
    GpuBuffer ConstantBuffer;
    Camera3D Camera;
    MeshDraw* Draws; // In the engine's Lower heap
    uint32 DrawCount;
};
static Renderer3D Renderer3DData;

bool Renderer3DInit(StackAllocator* allocator, RendererInitError* error)
{
    // NOTE(saeb): Until the game sets one: at the origin, looking along +z.
    Renderer3DData.Camera.FieldOfView = DirectX::XMConvertToRadians(70.0f);
    Renderer3DData.Camera.Near = 0.1f;

    error->Message = SV8(u8"Couldn't set up the 3D renderer.");
    error->Reason = AssetDescribeResult(AssetLoadResult::OutOfMemory);

    Renderer3DData.Draws = (MeshDraw*)Allocate(allocator, Heap::Lower, AG_MAX_MESH_DRAWS * sizeof(MeshDraw), alignof(MeshDraw));
    if(!Renderer3DData.Draws)
    {
        return(false);
    }

    error->Reason = AssetDescribeResult(AssetLoadResult::RendererFailed);

    GpuBufferDesc constantBufferDesc = {};
    constantBufferDesc.Type = GpuBufferType::Constant;
    constantBufferDesc.Usage = GpuUsage::Dynamic;
    constantBufferDesc.Size = sizeof(MeshConstants);
    constantBufferDesc.DebugName = SV8(u8"MeshConstants");

    Renderer3DData.ConstantBuffer = GpuCreateBuffer(&constantBufferDesc);
    if(!Renderer3DData.ConstantBuffer.Object)
    {
        return(false);
    }

    // NOTE(saeb): The mesh shader every mesh draws with; the driver keeps its own copy of the bytecode, so the file is scratch.
    Frame scratch = GetFrame(allocator, Heap::Upper);

    AssetShader shader;
    AssetLoadResult shaderResult = AssetReadShader(allocator, SV8(AG_MESH_SHADER_PATH), &shader);
    if(shaderResult == AssetLoadResult::Ok && (!shader.Vertex || !shader.Pixel))
    {
        shaderResult = AssetLoadResult::MissingStage;
    }

    if(shaderResult == AssetLoadResult::Ok)
    {
        GpuPipelineDesc desc = {};
        desc.VertexShader = shader.Vertex;
        desc.VertexShaderSize = shader.VertexSize;
        desc.PixelShader = shader.Pixel;
        desc.PixelShaderSize = shader.PixelSize;
        desc.Attributes[0] = { 0, GpuVertexFormat::Float3, offsetof(Renderer3DVertex, X) };
        desc.Attributes[1] = { 1, GpuVertexFormat::Float3, offsetof(Renderer3DVertex, NX) };
        desc.AttributeCount = 2;
        desc.Blend = GpuBlend::Opaque;
        desc.Cull = GpuCull::Back; // A closed mesh never shows its inside, so skip faces turned away from the camera
        desc.Depth = GpuDepth::TestWrite;
        desc.DebugName = SV8(u8"MeshPipeline");

        Renderer3DData.Pipeline = GpuCreatePipeline(allocator, &desc);
    }

    ReleaseFrame(allocator, scratch);

    if(!Renderer3DData.Pipeline.Object)
    {
        error->Message = SV8(u8"Couldn't load the mesh shader (" AG_MESH_SHADER_PATH u8").");
        error->Reason = (shaderResult != AssetLoadResult::Ok) ? AssetDescribeResult(shaderResult) : AssetDescribeResult(AssetLoadResult::RendererFailed);
        return(false);
    }

    return(true);
}

void Renderer3DEndFrame(bool draw)
{
    if(draw && Renderer3DData.DrawCount > 0)
    {
        GpuBeginMarker(SV8(u8"Meshes"));

        int32 backBufferWidth, backBufferHeight;
        GpuGetBackBufferSize(&backBufferWidth, &backBufferHeight);

        // NOTE(saeb): The same for every draw this frame: the camera, and one sun up high, behind the left shoulder of a camera looking along +z.
        MeshConstants constants = {};
        DirectX::XMStoreFloat4x4(&constants.ViewProjection, Camera3DGetViewProjection(&Renderer3DData.Camera, backBufferWidth, backBufferHeight));
        DirectX::XMFLOAT3 toSun;
        DirectX::XMStoreFloat3(&toSun, DirectX::XMVector3Normalize(DirectX::XMVectorSet(-0.5f, 0.8f, -0.3f, 0.0f)));
        constants.ToSun[0] = toSun.x; constants.ToSun[1] = toSun.y; constants.ToSun[2] = toSun.z;
        constants.SunColor[0] = 1.0f; constants.SunColor[1] = 0.95f; constants.SunColor[2] = 0.85f;
        constants.AmbientColor[0] = 0.35f; constants.AmbientColor[1] = 0.4f; constants.AmbientColor[2] = 0.5f;

        GpuSetPipeline(Renderer3DData.Pipeline);
        GpuSetConstantBuffer(0, Renderer3DData.ConstantBuffer);

        for(uint32 index = 0; index < Renderer3DData.DrawCount; ++index)
        {
            const MeshDraw* meshDraw = &Renderer3DData.Draws[index];

            constants.World = meshDraw->World;
            constants.Color[0] = meshDraw->R; constants.Color[1] = meshDraw->G; constants.Color[2] = meshDraw->B; constants.Color[3] = 1.0f;

            // NOTE(saeb): Rewritten for every draw; Map discards the old contents, so the GPU keeps the previous draw's copy while this one fills.
            MeshConstants* mapped = (MeshConstants*)GpuMapBuffer(Renderer3DData.ConstantBuffer);
            if(!mapped)
            {
                continue;
            }
            *mapped = constants;
            GpuUnmapBuffer(Renderer3DData.ConstantBuffer);

            GpuSetVertexBuffer(meshDraw->Mesh.Vertices, sizeof(Renderer3DVertex));
            GpuSetIndexBuffer(meshDraw->Mesh.Indices, GpuIndexFormat::U32);
            GpuDrawIndexed(meshDraw->Mesh.IndexCount, 0);
        }

        GpuEndMarker();
    }

    Renderer3DData.DrawCount = 0;
}

void Renderer3DShutdown()
{
    GpuDestroyPipeline(Renderer3DData.Pipeline);
    GpuDestroyBuffer(Renderer3DData.ConstantBuffer);
    Renderer3DData.Pipeline = {};
    Renderer3DData.ConstantBuffer = {};

    // NOTE(saeb): The draw list lives in the engine's Lower heap; ShutdownStackAllocator frees it.
    Renderer3DData.Draws = nullptr;
    Renderer3DData.DrawCount = 0;
}

bool Renderer3DCreateMesh(const Renderer3DVertex* vertices, uint32 vertexCount, const uint32* indices, uint32 indexCount, StringView8 debugName, Renderer3DMesh* mesh)
{
    *mesh = {};

    GpuBufferDesc vertexDesc = {};
    vertexDesc.Type = GpuBufferType::Vertex;
    vertexDesc.Usage = GpuUsage::Immutable;
    vertexDesc.Size = vertexCount * (uint32)sizeof(Renderer3DVertex);
    vertexDesc.Data = vertices;
    vertexDesc.DebugName = debugName;

    GpuBufferDesc indexDesc = {};
    indexDesc.Type = GpuBufferType::Index;
    indexDesc.Usage = GpuUsage::Immutable;
    indexDesc.Size = indexCount * (uint32)sizeof(uint32);
    indexDesc.Data = indices;
    indexDesc.DebugName = debugName;

    mesh->Vertices = GpuCreateBuffer(&vertexDesc);
    mesh->Indices = GpuCreateBuffer(&indexDesc);
    mesh->IndexCount = indexCount;

    if(!mesh->Vertices.Object || !mesh->Indices.Object)
    {
        Renderer3DDestroyMesh(mesh);
        return(false);
    }

    return(true);
}

void Renderer3DDestroyMesh(Renderer3DMesh* mesh)
{
    GpuDestroyBuffer(mesh->Vertices);
    GpuDestroyBuffer(mesh->Indices);
    *mesh = {};
}

void Renderer3DSetCamera(const Camera3D* camera)
{
    Renderer3DData.Camera = *camera;
}

void Renderer3DDrawMesh(const Renderer3DMesh* mesh, DirectX::FXMMATRIX world, real32 r, real32 g, real32 b)
{
    // NOTE(saeb): Like quads past the limit, extra draws are dropped rather than overflowing the list.
    if(Renderer3DData.DrawCount >= AG_MAX_MESH_DRAWS || !mesh->Vertices.Object)
    {
        return;
    }

    MeshDraw* meshDraw = &Renderer3DData.Draws[Renderer3DData.DrawCount++];
    meshDraw->Mesh = *mesh;
    DirectX::XMStoreFloat4x4(&meshDraw->World, world);
    meshDraw->R = r; meshDraw->G = g; meshDraw->B = b;
}
