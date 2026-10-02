#include "Engine/Renderer/Camera3D.h"

DirectX::XMFLOAT3 Camera3DGetForward(const Camera3D* camera)
{
    // NOTE(saeb): Yaw swings the direction around in the ground plane, pitch tilts it up; cos(pitch) shrinks the ground part as it tilts, so the length stays 1.
    real32 sinYaw, cosYaw, sinPitch, cosPitch;
    DirectX::XMScalarSinCos(&sinYaw, &cosYaw, camera->Yaw);
    DirectX::XMScalarSinCos(&sinPitch, &cosPitch, camera->Pitch);

    DirectX::XMFLOAT3 forward = { cosPitch * sinYaw, sinPitch, cosPitch * cosYaw };

    return(forward);
}

DirectX::XMMATRIX Camera3DGetViewProjection(const Camera3D* camera, int32 screenWidth, int32 screenHeight)
{
    // NOTE(saeb): The view moves the whole world so the eye sits at the origin looking down +z, with +y up. The GPU only ever sees the world from there.
    DirectX::XMFLOAT3 forward = Camera3DGetForward(camera);
    DirectX::XMVECTOR eye = DirectX::XMLoadFloat3(&camera->Position);
    DirectX::XMVECTOR direction = DirectX::XMLoadFloat3(&forward);
    DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    DirectX::XMMATRIX view = DirectX::XMMatrixLookToLH(eye, direction, up);

    real32 aspect = (screenWidth > 0 && screenHeight > 0) ? (real32)screenWidth / (real32)screenHeight : 1.0f;
    real32 nearPlane = (camera->Near > 0.0f) ? camera->Near : 0.1f;
    real32 fieldOfView = (camera->FieldOfView > 0.0f) ? camera->FieldOfView : DirectX::XMConvertToRadians(70.0f);

    // NOTE(saeb): At distance 1 from the eye, the top of the screen is tan(fov / 2) above the centre. Dividing by that puts the screen's edges at -1 and 1, which is where clip space ends. The width gets the same scale divided by the aspect, so the world isn't stretched.
    real32 sinHalf, cosHalf;
    DirectX::XMScalarSinCos(&sinHalf, &cosHalf, fieldOfView * 0.5f);
    real32 scaleY = cosHalf / sinHalf; // 1 / tan(fov / 2)
    real32 scaleX = scaleY / aspect;

    // NOTE(saeb): Perspective. The GPU divides x, y and z by w after the vertex shader; this matrix puts the distance (view z) into w, so far things shrink toward the centre. z becomes Near, so depth = Near / distance: 1 at the near plane, approaching 0 at the horizon, never reaching it. That's reversed depth with no far plane.
    DirectX::XMMATRIX projection = DirectX::XMMatrixSet(scaleX, 0.0f, 0.0f, 0.0f,
                                                        0.0f, scaleY, 0.0f, 0.0f,
                                                        0.0f, 0.0f, 0.0f, 1.0f,
                                                        0.0f, 0.0f, nearPlane, 0.0f);

    // NOTE(saeb): Row vectors, so the order reads left to right: first into the camera's view, then through the lens.
    return(DirectX::XMMatrixMultiply(view, projection));
}
