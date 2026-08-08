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
    GameMemory* memory;
    GameContext context = {};

    if(PlatformInit(WindowTitle, 1280, 720, &memory))
    {
        context.Memory = memory;

        WindowDimensions dims;
        Win32GetWindowDimensions(&dims);

        D3D11RendererState renderer = {};
        if(D3D11InitRenderer(&renderer, Win32GetWindowHandle(), dims.Width, dims.Height, &memory->Engine, &memory->Transient, 16384))
        {
            RenderCommands renderCommands = {};
            renderCommands.MaxQuads = 16384;
            renderCommands.Quads = sstl::PushArray<RenderCommandQuad>(&memory->Engine, renderCommands.MaxQuads);

            GameAssets gameAssets = {};
            AssetManager assetManager = {};
            AssetManagerInit(&assetManager, &renderer);

            GameInput input = {};

            context.Render = &renderCommands;
            context.Assets = &gameAssets;
            context.Loader = &assetManager;
            context.Input = &input;

            uint64 clockFrequency = PlatformGetClockFrequency();
            uint64 lastCounter = PlatformGetClockTicks();

            GameInit(&context);

            Win32ShowWindow();
            bool8 running = true;
            while(running)
            {
                Win32BeginInputFrame(&input);
                PlatformPumpEvents(&input);

                if(Win32WindowShouldQuit())
                {
                    running = false;
                    break;
                }

                if(Win32WindowConsumeResize(&dims))
                {
                    D3D11ResizeRenderer(&renderer, dims.Width, dims.Height);
                }

                memory->Transient.Used = 0;
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

                uint64 endCounter = PlatformGetClockTicks();
                context.DeltaTime = PlatformGetSecondsElapsed(clockFrequency, lastCounter, endCounter);

                real32 maxDeltaTime = 0.1f;
                if(context.DeltaTime > maxDeltaTime)
                {
                    context.DeltaTime = maxDeltaTime;
                }

                lastCounter = endCounter;
            }

            D3D11ShutdownRenderer(&renderer);
            PlatformShutdown();
        }
        else
        {
            PlatformShutdown();
            EMB("Renderer initialization failed!");
        }
    }
    else
    {
        EMB("Platform initialization failed!");
    }

    return(0);
}
