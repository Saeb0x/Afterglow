#if !defined(D3D11RENDERER_H)

#include "Core/Types.h"

#include <windows.h>

#include <d3d11.h>
#include <dxgi1_6.h>

struct D3D11RendererState
{
    ID3D11Device* Device;
    ID3D11DeviceContext* Context;
    IDXGISwapChain1* SwapChain;
    ID3D11RenderTargetView* RenderTargetView;
    ID3D11Texture2D* DepthStencilBuffer;
    ID3D11DepthStencilView* DepthStencilView;
};

bool32 D3D11InitRenderer(D3D11RendererState* renderer, HWND windowHandle, int32 width, int32 height);
void D3D11ResizeRenderer(D3D11RendererState* renderer, int32 width, int32 height);
void D3D11ShutdownRenderer(D3D11RendererState* renderer);

void D3D11BeginFrame(D3D11RendererState* renderer);
void D3D11Present(D3D11RendererState* renderer);

#define D3D11RENDERER_H
#endif
