#include "Camera.h"

#include "Engine/Platform/Window.h"

// NOTE(saeb): The world size the window shows: the extent fitted entirely inside the window, as large as it allows, then divided by the zoom.
static void CameraGetVisibleSize(const Camera* camera, real32 screenWidth, real32 screenHeight, real32* visibleWidth, real32* visibleHeight)
{
    // NOTE(saeb): No window area to fit: show the extent as it is (or one metre), so callers still get finite numbers.
    if(screenWidth <= 0.0f || screenHeight <= 0.0f)
    {
        *visibleWidth = (camera->Extent.x > 0.0f) ? camera->Extent.x : 1.0f;
        *visibleHeight = (camera->Extent.y > 0.0f) ? camera->Extent.y : 1.0f;
        return;
    }

    real32 extentX = (camera->Extent.x > 0.0f) ? camera->Extent.x : screenWidth; // No extent: one metre per pixel
    real32 extentY = (camera->Extent.y > 0.0f) ? camera->Extent.y : screenHeight;
    real32 zoom = (camera->Zoom > 0.0f) ? camera->Zoom : 1.0f;

    real32 scaleX = screenWidth / extentX;
    real32 scaleY = screenHeight / extentY;
    real32 pixelsPerMetre = ((scaleX < scaleY) ? scaleX : scaleY) * zoom;

    *visibleWidth = screenWidth / pixelsPerMetre;
    *visibleHeight = screenHeight / pixelsPerMetre;
}

DirectX::XMMATRIX CameraGetViewProjection(const Camera* camera, int32 screenWidth, int32 screenHeight)
{
    real32 visibleWidth, visibleHeight;
    CameraGetVisibleSize(camera, (real32)screenWidth, (real32)screenHeight, &visibleWidth, &visibleHeight);

    // NOTE(saeb): Row vectors, so the order reads left to right: move the camera's position to the origin, turn the world the opposite way to the camera, then fit the visible size to clip space. OrthographicLH maps y up to up, so no flip is needed.
    DirectX::XMMATRIX view = DirectX::XMMatrixMultiply(DirectX::XMMatrixTranslation(-camera->Position.x, -camera->Position.y, 0.0f), DirectX::XMMatrixRotationZ(-camera->Rotation));
    DirectX::XMMATRIX projection = DirectX::XMMatrixOrthographicLH(visibleWidth, visibleHeight, 0.0f, 1.0f);

    return(DirectX::XMMatrixMultiply(view, projection));
}

DirectX::XMFLOAT2 CameraScreenToWorld(const Camera* camera, int32 screenX, int32 screenY)
{
    int32 screenWidth, screenHeight;
    WindowGetClientAreaDimensions(&screenWidth, &screenHeight);

    real32 visibleWidth, visibleHeight;
    CameraGetVisibleSize(camera, (real32)screenWidth, (real32)screenHeight, &visibleWidth, &visibleHeight);

    // NOTE(saeb): The pixel's centre (+ 0.5), as an offset from the window's centre in metres, with y flipped to point up.
    real32 offsetX = (((real32)screenX + 0.5f) / (real32)screenWidth - 0.5f) * visibleWidth;
    real32 offsetY = (0.5f - ((real32)screenY + 0.5f) / (real32)screenHeight) * visibleHeight;

    // Turn the offset with the camera, then add the camera's position.
    real32 sine, cosine;
    DirectX::XMScalarSinCos(&sine, &cosine, camera->Rotation);

    DirectX::XMFLOAT2 world;
    world.x = camera->Position.x + offsetX * cosine - offsetY * sine;
    world.y = camera->Position.y + offsetX * sine + offsetY * cosine;

    return(world);
}

DirectX::XMFLOAT2 CameraWorldToScreen(const Camera* camera, DirectX::XMFLOAT2 world)
{
    int32 screenWidth, screenHeight;
    WindowGetClientAreaDimensions(&screenWidth, &screenHeight);

    real32 visibleWidth, visibleHeight;
    CameraGetVisibleSize(camera, (real32)screenWidth, (real32)screenHeight, &visibleWidth, &visibleHeight);

    // The exact reverse: offset from the camera, turned back the other way, then from metres to pixels.
    real32 sine, cosine;
    DirectX::XMScalarSinCos(&sine, &cosine, -camera->Rotation);

    real32 relativeX = world.x - camera->Position.x;
    real32 relativeY = world.y - camera->Position.y;
    real32 offsetX = relativeX * cosine - relativeY * sine;
    real32 offsetY = relativeX * sine + relativeY * cosine;

    DirectX::XMFLOAT2 screen;
    screen.x = (offsetX / visibleWidth + 0.5f) * (real32)screenWidth;
    screen.y = (0.5f - offsetY / visibleHeight) * (real32)screenHeight;

    return(screen);
}

void CameraGetVisibleBounds(const Camera* camera, real32* minX, real32* minY, real32* maxX, real32* maxY)
{
    int32 screenWidth, screenHeight;
    WindowGetClientAreaDimensions(&screenWidth, &screenHeight);

    real32 visibleWidth, visibleHeight;
    CameraGetVisibleSize(camera, (real32)screenWidth, (real32)screenHeight, &visibleWidth, &visibleHeight);

    // NOTE(saeb): A rotated rectangle's bounding box: each half-side contributes along both axes by the absolute sine and cosine.
    real32 sine, cosine;
    DirectX::XMScalarSinCos(&sine, &cosine, camera->Rotation);
    sine = (sine < 0.0f) ? -sine : sine;
    cosine = (cosine < 0.0f) ? -cosine : cosine;

    real32 halfWidth = visibleWidth * 0.5f;
    real32 halfHeight = visibleHeight * 0.5f;
    real32 boundX = halfWidth * cosine + halfHeight * sine;
    real32 boundY = halfWidth * sine + halfHeight * cosine;

    *minX = camera->Position.x - boundX;
    *maxX = camera->Position.x + boundX;
    *minY = camera->Position.y - boundY;
    *maxY = camera->Position.y + boundY;
}
