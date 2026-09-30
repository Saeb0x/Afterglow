#if !defined(AFTERGLOW_D3D11GPU_H)
#define AFTERGLOW_D3D11GPU_H

#include <SSTL/Core/String.h>

#include <d3d11.h>
#include <d3d11_1.h>

ID3D11Device* D3D11GpuGetDevice();
ID3D11DeviceContext* D3D11GpuGetContext();
ID3DUserDefinedAnnotation* D3D11GpuGetAnnotation();
void D3D11SetName(ID3D11DeviceChild* object, StringView8 name);

#endif
