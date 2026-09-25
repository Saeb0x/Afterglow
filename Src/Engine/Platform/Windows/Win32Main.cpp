#include "Win32Window.h"
#include "Engine/Platform/Window.h"
#include "Win32Time.h"
#include "Engine/Renderer/D3D11/D3D11Renderer.h"
#include "Engine/Asset/Asset.h"

#include "Game/Game.h"

#include <SSTL/Core/Utility.h>

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#define AG_DEFAULT_PIPELINE_PATH u8"Data/Engine/Quad.aga"

static StackAllocator EngineMemory;

// NOTE(saeb): Startup failures happen before the window is shown, so without a message the game would simply never appear. Builds "message\n\nReason: reason" in scratch on both heaps, released before returning.
static void Win32ShowStartupError(StackAllocator* allocator, StringView8 message, StringView8 reason)
{
    Frame lowerScratch = GetFrame(allocator, Heap::Lower);
    Frame upperScratch = GetFrame(allocator, Heap::Upper);

    StringView8 separator = SV8(u8"\n\nReason: ");
    String8 text = String8Reserve(allocator, message.Length + separator.Length + reason.Length + 1);
    String8Append(&text, message);
    if(reason.Length > 0)
    {
        String8Append(&text, separator);
        String8Append(&text, reason);
    }

    StringView16 wideText = SV8ToSV16(allocator, StringView8{ text.Data, text.Length });
    const wchar_t* shownText = (wideText.Data && wideText.Length > 0) ? (const wchar_t*)wideText.Data : L"Afterglow failed to start.";
    MessageBoxW(nullptr, shownText, L"Afterglow", MB_OK | MB_ICONERROR);

    ReleaseFrame(allocator, upperScratch);
    ReleaseFrame(allocator, lowerScratch);
}

int APIENTRY WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int showCommand)
{
    // NOTE(saeb): Only a clean run through the main loop counts as success; every startup failure exits with 1.
    int exitCode = 1;

    if(InitStackAllocator(&EngineMemory, SSTL_MIB(64)))
    {
        GameConfigure();

        if(Win32WindowCreate(&EngineMemory, SV8(u8"Afterglow"), 1280, 720))
        {
            if(D3D11RendererInit(&EngineMemory, Win32WindowGetHandle()))
            {
                AssetLoadResult pipelineResult = AssetLoadDefaultPipeline(&EngineMemory, SV8(AG_DEFAULT_PIPELINE_PATH));
                if(pipelineResult == AssetLoadResult::Ok)
                {
                    if(GameInit(&EngineMemory))
                    {
                        ShowWindow(Win32WindowGetHandle(), SW_SHOW);

                        Win32TimeInit();

                        while(Win32WindowPumpEvents())
                        {
                            // NOTE(saeb): Nothing to show while minimized; sleep until a message (restore, quit) arrives instead of spinning.
                            if(WindowGetMinimized())
                            {
                                WaitMessage();
                                continue;
                            }

                            Frame frameScratch = GetFrame(&EngineMemory, Heap::Upper);

                            uint32 windowClientAreaWidth, windowClientAreaHeight;
                            WindowGetClientAreaDimensions(&windowClientAreaWidth, &windowClientAreaHeight);

                            D3D11RendererBeginFrame(windowClientAreaWidth, windowClientAreaHeight);
                            GameUpdate(&EngineMemory, Win32TimeTick());
                            D3D11RendererEndFrame();

                            ReleaseFrame(&EngineMemory, frameScratch);
                        }

                        GameShutdown(&EngineMemory);

                        exitCode = 0;
                    }
                    else
                    {
                        Win32ShowStartupError(&EngineMemory, SV8(u8"The game failed to initialize."), StringView8{ nullptr, 0 });
                    }
                }
                else
                {
                    Win32ShowStartupError(&EngineMemory, SV8(u8"Couldn't load the default shader (" AG_DEFAULT_PIPELINE_PATH u8")."), AssetDescribeResult(pipelineResult));
                }
            }
            else
            {
                Win32ShowStartupError(&EngineMemory, SV8(u8"Couldn't initialize Direct3D 11. Afterglow needs Windows 10 or later and a GPU with Direct3D feature level 11.0."), StringView8{ nullptr, 0 });
            }

            // NOTE(saeb): Called even if Init failed; Init can fail halfway, and Shutdown only releases what exists.
            D3D11RendererShutdown();

            Win32WindowShutdown();
        }
        else
        {
            Win32ShowStartupError(&EngineMemory, SV8(u8"Couldn't create the window."), StringView8{ nullptr, 0 });
        }

        ShutdownStackAllocator(&EngineMemory);
    }
    else
    {
        // NOTE(saeb): No allocator to build a message with, so a fixed one.
        MessageBoxW(nullptr, L"Couldn't reserve memory to start.", L"Afterglow", MB_OK | MB_ICONERROR);
    }

    return(exitCode);
}
