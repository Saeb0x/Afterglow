#if !defined(AFTERGLOW_D3D11RENDERER_H)
#define AFTERGLOW_D3D11RENDERER_H

#include <SSTL/Core/Types.h>
#include <SSTL/Memory/StackAllocator.h>

bool D3D11RendererInit(StackAllocator* allocator);
void D3D11RendererBeginFrame(int32 width, int32 height);
void D3D11RendererEndFrame();
void D3D11RendererShutdown();

#endif
