#include "Engine/Game.h"

#include "Engine/Platform/Window.h"
#include "Engine/Platform/Input.h"
#include "Engine/Platform/Log.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Text.h"
#include "Engine/UI/UI.h"
#include "Engine/Asset/Asset.h"

#include <SSTL/Core/String.h>

#include <DirectXMath.h>

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
    DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationZ(DirectX::XM_PIDIV2);
    DirectX::XMVECTOR point = DirectX::XMVector2Transform(DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), rotation);
    DirectX::XMFLOAT2 result;
    DirectX::XMStoreFloat2(&result, point);

    char8 buffer[64];
    String8 text = { buffer, 0, sizeof(buffer) };
    String8Append(&text, SV8(u8"DirectXMath: "));
    String8AppendReal(&text, result.x, 2);
    String8Append(&text, SV8(u8", "));
    String8AppendReal(&text, result.y, 2);
    LogPrint(allocator, StringView8{ text.Data, text.Length });

    return(true);
}

void GameUpdate(StackAllocator* allocator, real64 deltaTime)
{
}

void GameShutdown(StackAllocator* allocator)
{
}
