#if !defined(AFTERGLOW_D3D11RENDERER_H)
#define AFTERGLOW_D3D11RENDERER_H

#include <SSTL/Core/Types.h>

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

bool D3D11RendererInit(HWND windowHandle);
void D3D11RendererBeginFrame(uint32 width, uint32 height);
void D3D11RendererEndFrame();
void D3D11RendererShutdown();

#endif
