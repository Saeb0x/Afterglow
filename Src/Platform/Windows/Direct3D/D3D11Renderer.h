#if !defined(D3D11RENDERER_H)

#include "Core/Types.h"

#include <windows.h>

#include <d3d11.h>
#include <dxgi1_6.h>

bool D3D11InitRenderer(HWND windowHandle, int32 width, int32 height);
void D3D11ResizeRenderer(int32 width, int32 height);
void D3D11ShutdownRenderer();

void D3D11BeginFrame();
void D3D11Present();

#define D3D11RENDERER_H
#endif
