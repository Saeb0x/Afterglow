#include "Engine/Game.h"

#include "Engine/Platform/Window.h"
#include "Engine/Platform/Input.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Text.h"
#include "Engine/UI/UI.h"
#include "Engine/Asset/Asset.h"

#include <SSTL/Core/String.h>

#include <DirectXMath.h>

#define SANDBOX_MAX_BOXES 64

static Font DebugFont;
static UIContext DebugUI;
static UIPanel CameraPanel = { 20.0f, 20.0f, 320.0f, 300.0f, 320.0f, 300.0f };

static Camera WorldCamera = { { 0.0f, 0.0f }, { 20.0f, 20.0f }, 1.0f, 0.0f };

static DirectX::XMFLOAT2 Boxes[SANDBOX_MAX_BOXES]; // Centres of the boxes placed by clicking
static uint32 BoxCount;

static void SandboxRect(real32 x, real32 y, real32 width, real32 height, real32 r, real32 g, real32 b)
{
    RendererQuad quad = {};
    quad.X = x; quad.Y = y; quad.Width = width; quad.Height = height;
    quad.R = r; quad.G = g; quad.B = b; quad.A = 1.0f;
    RendererPushQuad(&quad);
}

static void SandboxLabelXY(StringView8 label, real32 x, real32 y)
{
    char8 buffer[64];
    String8 line = { buffer, 0, sizeof(buffer) };
    String8Append(&line, label);
    String8AppendReal(&line, x, 2);
    String8Append(&line, SV8(u8", "));
    String8AppendReal(&line, y, 2);

    UILabel(&DebugUI, StringView8{ line.Data, line.Length });
}

void GameConfigure()
{
    WindowSetFlags(WindowFlags_None);
    WindowSetTitle(SV8(u8"Afterglow Sandbox"));
    WindowSetClientAreaDimensions(1280, 720);
    WindowSetMinClientAreaDimensions(360, 360);
    InputSetFlags(InputFlags_Mouse | InputFlags_Keyboard);
    RendererSetFlags(RendererFlags_VSync);
}

bool GameInit(StackAllocator* allocator)
{
    AssetLoadFont(allocator, SV8(u8"Data/Engine/LiberationMono-Regular.aga"), &DebugFont);

    return(true);
}

void GameUpdate(StackAllocator* allocator, real64 deltaTime)
{
    // Pan along the screen's directions: "right on screen" is the camera's rotated x axis. 10 metres per second at zoom 1.
    real32 sine, cosine;
    DirectX::XMScalarSinCos(&sine, &cosine, WorldCamera.Rotation);
    real32 moveX = (real32)(InputKeyDown(InputKey::Right) - InputKeyDown(InputKey::Left));
    real32 moveY = (real32)(InputKeyDown(InputKey::Up) - InputKeyDown(InputKey::Down));
    real32 step = 10.0f * (real32)deltaTime / WorldCamera.Zoom;
    WorldCamera.Position.x += (moveX * cosine - moveY * sine) * step;
    WorldCamera.Position.y += (moveX * sine + moveY * cosine) * step;

    RendererSetCamera(&WorldCamera);

    // Background over everything the window shows, from the camera's visible bounds.
    real32 minX, minY, maxX, maxY;
    CameraGetVisibleBounds(&WorldCamera, &minX, &minY, &maxX, &maxY);
    SandboxRect(minX, minY, maxX - minX, maxY - minY, 0.12f, 0.13f, 0.16f);

    // Grid lines every 5 metres, 0.05 m thick.
    for(int32 line = -20; line <= 20; ++line)
    {
        real32 at = (real32)line * 5.0f;
        SandboxRect(at - 0.025f, -100.0f, 0.05f, 200.0f, 0.22f, 0.23f, 0.27f);
        SandboxRect(-100.0f, at - 0.025f, 200.0f, 0.05f, 0.22f, 0.23f, 0.27f);
    }

    // Axes: +x red, +y green, 5 metres long. And a 1 m box whose bottom-left is the origin.
    SandboxRect(0.0f, -0.05f, 5.0f, 0.1f, 0.9f, 0.2f, 0.2f);
    SandboxRect(-0.05f, 0.0f, 0.1f, 5.0f, 0.2f, 0.9f, 0.2f);
    SandboxRect(0.0f, 0.0f, 1.0f, 1.0f, 0.9f, 0.8f, 0.2f);

    // Placed boxes, 0.5 m, centred where they were clicked.
    for(uint32 index = 0; index < BoxCount; ++index)
    {
        SandboxRect(Boxes[index].x - 0.25f, Boxes[index].y - 0.25f, 0.5f, 0.5f, 0.3f, 0.6f, 1.0f);
    }

    int32 mouseX, mouseY;
    InputGetMouseXY(&mouseX, &mouseY);
    DirectX::XMFLOAT2 mouseWorld = CameraScreenToWorld(&WorldCamera, mouseX, mouseY);

    UIBegin(&DebugUI, &DebugFont);
    UIPanelBegin(&DebugUI, &CameraPanel, SV8(u8"Camera"));

    SandboxLabelXY(SV8(u8"Position: "), WorldCamera.Position.x, WorldCamera.Position.y);
    SandboxLabelXY(SV8(u8"Mouse: "), mouseWorld.x, mouseWorld.y);
    UISlider(&DebugUI, SV8(u8"Zoom"), &WorldCamera.Zoom, 0.25f, 4.0f);
    UISlider(&DebugUI, SV8(u8"Rotation"), &WorldCamera.Rotation, -DirectX::XM_PI, DirectX::XM_PI);

    if(UIButton(&DebugUI, SV8(u8"Reset")))
    {
        WorldCamera.Position = { 0.0f, 0.0f };
        WorldCamera.Zoom = 1.0f;
        WorldCamera.Rotation = 0.0f;
        BoxCount = 0;
    }

    UIPanelEnd(&DebugUI);
    UIEnd(&DebugUI);

    // Click in the world, not on the panel: drop a box at the mouse. After UIEnd, so UIWantsMouse describes this frame.
    if(InputMouseButtonPressed(InputMouseButton::Left) && !UIWantsMouse(&DebugUI) && BoxCount < SANDBOX_MAX_BOXES)
    {
        Boxes[BoxCount++] = mouseWorld;
    }
}

void GameShutdown(StackAllocator* allocator)
{
}
