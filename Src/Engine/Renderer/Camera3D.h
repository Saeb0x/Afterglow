#if !defined(AFTERGLOW_CAMERA3D_H)
#define AFTERGLOW_CAMERA3D_H

#include <SSTL/Core/Types.h>

#include <DirectXMath.h>

// NOTE(saeb): A first-person eye in a 3D world. World units are metres: +x right, +y up, +z forward (away from the viewer at yaw 0).
struct Camera3D
{
    DirectX::XMFLOAT3 Position;
    real32 Yaw; // Radians around the up axis; 0 looks along +z, positive turns right
    real32 Pitch; // Radians above (+) or below (-) the horizon; keep it short of straight up or down
    real32 FieldOfView; // Vertical, in radians; about 70 degrees feels natural
    real32 Near; // Anything closer than this is cut away
};

// NOTE(saeb): The unit-length direction the camera looks in, for moving forward and aiming.
DirectX::XMFLOAT3 Camera3DGetForward(const Camera3D* camera);

// NOTE(saeb): World metres to clip space for a screen of the given size in pixels. Reversed depth with no far plane: depth is Near / distance, 1 at the near plane and falling toward 0 at the horizon.
DirectX::XMMATRIX Camera3DGetViewProjection(const Camera3D* camera, int32 screenWidth, int32 screenHeight);

#endif
