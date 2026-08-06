#include "Game/Afterglow.cpp"

#include "Win32Utils.h"
#include "Window/Win32Window.cpp"
#include "Direct3D/D3D11Renderer.cpp"
#include "IO/Win32File.cpp"
#include "Font/Win32Font.cpp"
#include "Texture/Win32Texture.cpp"
#include "Time/Win32Time.cpp"
#include "Assets/Win32AssetManager.cpp"
#include "Input/Win32Input.cpp"

static const char* WindowTitle =
#if defined(AG_DEBUG)
    "Afterglow - DX11 | Debug";
#else
    "Afterglow";
#endif

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int)
{
    uint64 engineArenaSize = 16 * sstl::Megabytes;
    uint64 permanentArenaSize = 64 * sstl::Megabytes;
    uint64 transientArenaSize = 1 * sstl::Gigabytes;
    uint64 totalSize = engineArenaSize + permanentArenaSize + transientArenaSize;

    void* memoryBlock = (void*)VirtualAlloc(0, totalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    sstl::Arena engineMemory = {};
    GameMemory gameMemory = {};
    sstl::InitializeArena(&engineMemory, memoryBlock, engineArenaSize);
    sstl::InitializeArena(&gameMemory.Permanent, (uint8*)memoryBlock + engineArenaSize, permanentArenaSize);
    sstl::InitializeArena(&gameMemory.Transient, (uint8*)memoryBlock + engineArenaSize + permanentArenaSize, transientArenaSize);

    if(engineMemory.Base)
    {
        HWND windowHandle;
        if(Win32CreateWindow(instance, WindowTitle, 1280, 720, &windowHandle))
        {
            WindowDimensions dims;
            Win32GetWindowDimensions(windowHandle, &dims);

            D3D11RendererState renderer = {};
            if(D3D11InitRenderer(&renderer, windowHandle, dims.Width, dims.Height, &engineMemory, &gameMemory.Transient, 16384))
            {
                RenderCommands renderCommands = {};
                renderCommands.MaxQuads = 16384;
                renderCommands.Quads = sstl::PushArray<RenderCommandQuad>(&engineMemory, renderCommands.MaxQuads);

                GameAssets gameAssets = {};
                AssetManager assetManager = {};
                AssetManagerInit(&assetManager, &renderer);

                GameInput input = {};

                GameContext context = {};
                context.Memory = &gameMemory;
                context.Render = &renderCommands;
                context.Assets = &gameAssets;
                context.Loader = &assetManager;
                context.Input = &input;

                LARGE_INTEGER perfCounterFrequency = Win32GetPerformanceCounterFrequency();
                LARGE_INTEGER lastCounter = Win32GetPerformanceCounterTicks();

                GameInit(&context);
                
                Win32ShowWindow(windowHandle);
                bool32 running = true;
                while(running)
                {
                    Win32BeginInputFrame(&input);
                    Win32ProcessPendingMessages(&input);

                    if(Win32WindowShouldQuit())
                    {
                        running = false;
                        break;
                    }

                    if(Win32WindowConsumeResize(&dims))
                    {
                        D3D11ResizeRenderer(&renderer, dims.Width, dims.Height);
                    }

                    gameMemory.Transient.Used = 0;
                    if(!Win32WindowIsMinimized())
                    {
                        D3D11BeginFrame(&renderer);
                        D3D11BeginQuadPass(&renderer, dims.Width, dims.Height);
                        renderCommands.QuadCount = 0;
                        renderCommands.CurrentLayer = 0;
                        renderCommands.CurrentMaterial = 0;

                        context.ScreenWidth = dims.Width;
                        context.ScreenHeight = dims.Height;

                        GameUpdateAndRender(&context);

                        D3D11SubmitRenderCommands(&renderer, &renderCommands);
                        D3D11EndQuadPass(&renderer);
                        D3D11Present(&renderer);
                    }

                    LARGE_INTEGER endCounter = Win32GetPerformanceCounterTicks();
                    context.DeltaTime = Win32GetSecondsElapsed(perfCounterFrequency, lastCounter, endCounter);

                    real32 maxDeltaTime = 0.1f;
                    if(context.DeltaTime > maxDeltaTime)
                    {
                        context.DeltaTime = maxDeltaTime;
                    }

                    lastCounter = endCounter;
                }

                D3D11ShutdownRenderer(&renderer);
                VirtualFree(engineMemory.Base, 0, MEM_RELEASE);
            }
            else
            {
                EMB("Renderer initialization failed!");
            }
        }
        else
        {
            EMB("Window creation failed!");
        }
    }
    else
    {
        EMB("Game memory allocation failed!");
    }

    return(0);
}
