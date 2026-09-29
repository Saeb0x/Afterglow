#if !defined(AFTERGLOW_CAMERA_H)
#define AFTERGLOW_CAMERA_H

#include <SSTL/Core/Types.h>

#include <DirectXMath.h>

// NOTE(saeb): A view of the world. World units are metres with y up. The window always shows at least Extent / Zoom metres around Position, centred, with the window's extra length showing more world on its long sides.
struct Camera
{
    DirectX::XMFLOAT2 Position; // World metres at the centre of the window
    DirectX::XMFLOAT2 Extent; // Metres always visible at zoom 1; the window's extra length shows more
    real32 Zoom; // 1 = Extent fits; 2 = twice as close
    real32 Rotation; // Radians, counter-clockwise; the world turns the other way on screen
};

// NOTE(saeb): World metres to clip space for a screen of the given size in pixels. The renderer calls this; games use the conversions below.
DirectX::XMMATRIX CameraGetViewProjection(const Camera* camera, int32 screenWidth, int32 screenHeight);

// NOTE(saeb): Screen pixels (client area, y down, as from InputGetMouseXY) to world metres, and back. Both use the window's current size.
DirectX::XMFLOAT2 CameraScreenToWorld(const Camera* camera, int32 screenX, int32 screenY);
DirectX::XMFLOAT2 CameraWorldToScreen(const Camera* camera, DirectX::XMFLOAT2 world);

// NOTE(saeb): The smallest world-aligned rectangle covering everything the window shows; larger than the view when rotated. For backgrounds and culling.
void CameraGetVisibleBounds(const Camera* camera, real32* minX, real32* minY, real32* maxX, real32* maxY);

#endif
