#include "Engine/Platform/Windows/Win32Window.h"
#include "Engine/Platform/Window.h"
#include "Engine/Platform/Windows/Win32Time.h"
#include "Engine/Asset/Asset.h"
#include "Engine/Renderer/Internal/GpuInternal.h"
#include "Engine/Renderer/Internal/RendererInternal.h"
#include "Engine/Game.h"

#include <SSTL/Core/Utility.h>

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#define AG_TEXT_PIPELINE_PATH u8"Data/Engine/Text.aga"

static StackAllocator EngineMemory;

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
    int exitCode = 1;

    if(InitStackAllocator(&EngineMemory, SSTL_MIB(64)))
    {
        GameConfigure();

        if(Win32WindowCreate(&EngineMemory))
        {
            GpuDesc gpuDesc = {};
            gpuDesc.WindowHandle = Win32WindowGetHandle();
            GpuInitResult gpuResult = GpuInit(&gpuDesc);
            RendererInitError rendererError = {};

            if(gpuResult != GpuInitResult::Ok)
            {
                switch(gpuResult)
                {
                    case GpuInitResult::NoGpu:
                    {
                        Win32ShowStartupError(&EngineMemory, SV8(u8"Couldn't find a GPU. Afterglow needs a hardware GPU with Direct3D 11 support."), StringView8{ nullptr, 0 });
                    } break;

                    case GpuInitResult::Unsupported:
                    {
                        Win32ShowStartupError(&EngineMemory, SV8(u8"Your GPU isn't supported. Afterglow needs Windows 10 or later and a GPU with Direct3D feature level 11.0."), StringView8{ nullptr, 0 });
                    } break;

                    default:
                    {
                        Win32ShowStartupError(&EngineMemory, SV8(u8"Couldn't create the swap chain for the window."), StringView8{ nullptr, 0 });
                    } break;
                }
            }
            else if(RendererInit(&EngineMemory, &rendererError))
            {
                // NOTE(saeb): The text pipeline fonts draw with.
                Renderer2DPipeline textPipeline = 0;
                AssetLoadResult textPipelineResult = AssetLoadPipeline(&EngineMemory, SV8(AG_TEXT_PIPELINE_PATH), &textPipeline);

                if(textPipelineResult == AssetLoadResult::Ok)
                {
                    TextSetPipeline(textPipeline);

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

                            int32 windowClientAreaWidth, windowClientAreaHeight;
                            WindowGetClientAreaDimensions(&windowClientAreaWidth, &windowClientAreaHeight);

                            RendererBeginFrame(windowClientAreaWidth, windowClientAreaHeight);
                            GameUpdate(&EngineMemory, Win32TimeTick());
                            RendererEndFrame();

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
                    Win32ShowStartupError(&EngineMemory, SV8(u8"Couldn't load the text shader (" AG_TEXT_PIPELINE_PATH u8")."), AssetDescribeResult(textPipelineResult));
                }
            }
            else
            {
                Win32ShowStartupError(&EngineMemory, rendererError.Message, rendererError.Reason);
            }

            RendererShutdown();
            GpuShutdown();
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
        MessageBoxW(nullptr, L"Couldn't reserve memory to start.", L"Afterglow", MB_OK | MB_ICONERROR);
    }

    return(exitCode);
}
