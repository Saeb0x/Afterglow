#if !defined(D3D11RENDERER_H)

#include "Core/Types.h"
#include "Core/Arena.h"

#include <windows.h>

#include <d3d11.h>
#include <dxgi1_6.h>

#define MAX_UI_PASS_BATCHES 1024
#define MAX_TEXTURE_HANDLES 256

struct RenderCommands;

enum QuadKind
{
    SOLID,
    TEXTURED,
    GLYPH
};

struct Vertex2D
{
    real32 Position[2];
    real32 UV[2];
    uint32 Color;
};

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

struct D3D11UIPass
{
    // CPU-side accumulation.
    Vertex2D* Vertices;
    TextureBatch* Batches;
    uint32 MaxQuads;
    uint32 QuadCount;
    uint32 BatchCount;

    // GPU resources.
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

struct D3D11RendererState
{
    ID3D11Device* Device;
    ID3D11DeviceContext* Context;
    IDXGISwapChain1* SwapChain;
    ID3D11RenderTargetView* RenderTargetView;
    ID3D11Texture2D* DepthStencilBuffer;
    ID3D11DepthStencilView* DepthStencilView;

    D3D11UIPass UIPass;
    // NOTE(saeb): Later comes PBRPass and PostProcessPass.
};

bool32 D3D11InitRenderer(D3D11RendererState* renderer, HWND windowHandle, int32 width, int32 height, Arena* permanent, Arena* transient, uint32 maxQuads);
void D3D11ResizeRenderer(D3D11RendererState* renderer, int32 width, int32 height);
void D3D11ShutdownRenderer(D3D11RendererState* renderer);

void D3D11BeginFrame(D3D11RendererState* renderer);
void D3D11Present(D3D11RendererState* renderer);

void D3D11BeginUIPass(D3D11RendererState* renderer, int32 width, int32 height);
void D3D11SubmitRenderCommands(D3D11RendererState* renderer, RenderCommands* commands);
void D3D11EndUIPass(D3D11RendererState* renderer);

uint32 D3D11RegisterTexture(D3D11RendererState* renderer, ID3D11ShaderResourceView* texture, QuadKind kind);

#define D3D11RENDERER_H
#endif
