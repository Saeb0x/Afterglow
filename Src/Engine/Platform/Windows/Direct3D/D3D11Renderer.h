#if !defined(AFTERGLOW_D3D11RENDERER_H)
#define AFTERGLOW_D3D11RENDERER_H

#include "Engine/Types.h"
#include "SSTL/Memory.h"

#include <windows.h>

#include <d3d11.h>
#include <dxgi1_6.h>

#define MAX_QUAD_PASS_BATCHES 1024
#define MAX_TEXTURE_HANDLES 256
#define MAX_MATERIALS 64

struct RenderCommands;

struct Material
{
    ID3D11VertexShader* VertexShader;
    ID3D11PixelShader* PixelShader;
    ID3D11BlendState* BlendState;
    ID3D11DepthStencilState* DepthState;
    ID3D11RasterizerState* RasterizerState;
};

struct QuadVertex
{
    real32 Corner[2];
};

struct QuadInstance
{
    real32 Rect[4];
    real32 UV[4];
    uint32 Color;
};

struct DrawBatch
{
    uint32 MaterialHandle;
    ID3D11ShaderResourceView* Texture;
    uint32 InstanceCount;
};

struct TextureRegistryEntry
{
    ID3D11ShaderResourceView* Texture;
    uint32 MaterialHandle;
};

struct D3D11QuadPass
{
    QuadInstance* Instances;
    DrawBatch* Batches;
    uint32 MaxQuads;
    uint32 InstanceCount;
    uint32 BatchCount;

    ID3D11Buffer* UnitQuadBuffer;
    ID3D11Buffer* InstanceBuffer;
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
    uint32 StandardMaterial;
    uint32 GlyphMaterial;

    uint64* SortKeys;
    uint64* SortScratch;
};

struct D3D11RendererState
{
    ID3D11Device* Device;
    ID3D11DeviceContext* Context;
    IDXGISwapChain1* SwapChain;
    ID3D11RenderTargetView* RenderTargetView;
    ID3D11Texture2D* DepthStencilBuffer;
    ID3D11DepthStencilView* DepthStencilView;

    D3D11QuadPass QuadPass;
    // NOTE(saeb): Later comes PBRPass and PostProcessPass.

    Material Materials[MAX_MATERIALS];
    uint32 MaterialCount;
};

bool32 D3D11InitRenderer(D3D11RendererState* renderer, HWND windowHandle, int32 width, int32 height, sstl::Arena* permanent, sstl::Arena* transient, uint32 maxQuads);
void D3D11ResizeRenderer(D3D11RendererState* renderer, int32 width, int32 height);
void D3D11ShutdownRenderer(D3D11RendererState* renderer);

void D3D11BeginFrame(D3D11RendererState* renderer);
void D3D11Present(D3D11RendererState* renderer);

void D3D11BeginQuadPass(D3D11RendererState* renderer, int32 width, int32 height);
void D3D11SubmitRenderCommands(D3D11RendererState* renderer, RenderCommands* commands);
void D3D11EndQuadPass(D3D11RendererState* renderer);

uint32 D3D11RegisterTexture(D3D11RendererState* renderer, ID3D11ShaderResourceView* texture, uint32 materialHandle);
uint32 D3D11RegisterMaterial(D3D11RendererState* renderer, Material* material);

#endif
