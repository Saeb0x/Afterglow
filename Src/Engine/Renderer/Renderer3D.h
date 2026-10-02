#if !defined(AFTERGLOW_RENDERER3D_H)
#define AFTERGLOW_RENDERER3D_H

#include <SSTL/Core/Types.h>
#include <SSTL/Core/String.h>

#include "Engine/Renderer/Camera3D.h"
#include "Engine/Renderer/Gpu.h"

#include <DirectXMath.h>

// NOTE(saeb): One corner of a triangle. A flat-shaded mesh repeats a corner once for every face that meets there, because each face needs its own normal.
struct Renderer3DVertex
{
    real32 X, Y, Z; // Position, in the mesh's own metres
    real32 NX, NY, NZ; // Normal: the unit direction the surface faces
};

// NOTE(saeb): Triangles on the GPU. Corners listed clockwise when seen from outside, so the backs can be skipped.
struct Renderer3DMesh
{
    GpuBuffer Vertices;
    GpuBuffer Indices;
    uint32 IndexCount;
};

// NOTE(saeb): The GPU copies the vertices and indices, so the arrays can be scratch. Destroy the mesh when done with it.
bool Renderer3DCreateMesh(const Renderer3DVertex* vertices, uint32 vertexCount, const uint32* indices, uint32 indexCount, StringView8 debugName, Renderer3DMesh* mesh);
void Renderer3DDestroyMesh(Renderer3DMesh* mesh);

// NOTE(saeb): Meshes drawn this frame are seen through this camera; it's copied, and stays until another is set.
void Renderer3DSetCamera(const Camera3D* camera);

// NOTE(saeb): Draws the mesh at the end of the frame, before any 2D, moved into the world by world (mesh metres to world metres) and tinted by the colour. Rotation and uniform scale only; a stretched mesh would light wrongly.
void Renderer3DDrawMesh(const Renderer3DMesh* mesh, DirectX::FXMMATRIX world, real32 r, real32 g, real32 b);

#endif
