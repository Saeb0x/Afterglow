// NOTE(saeb): The engine's own test game: it defines the four functions Engine/Game.h declares, so the engine builds and runs without a real game. Games replace this file with their own.
#include "Engine/Game.h"

#include "Engine/Platform/Window.h"
#include "Engine/Platform/Input.h"
#include "Engine/Platform/Log.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Text.h"
#include "Engine/UI/UI.h"
#include "Engine/Asset/Asset.h"

#include <SSTL/Core/Config.h>

#if SSTL_DEBUG
static Font DebugFont;
static UIContext DebugUIContext;
static UIPanel DebugPanel = { 40.0f, 40.0f, 400.0f, 410.0f, 400.0f, 410.0f }; // Sized for the title bar and nine rows

// NOTE(saeb): Frame times are gathered over a short window and shown as its average and worst, so the numbers are readable instead of changing every frame.
#define SANDBOX_STATS_WINDOW 0.5 // Seconds

static real64 StatsElapsed; // Seconds gathered in the current window
static real64 StatsWorstDelta; // Longest frame in the current window
static uint32 StatsFrames;

static real64 StatsAverageMs, StatsWorstMs, StatsFPS; // The last completed window
static usize StatsUpperPeak; // Upper heap peak in the last completed window
static usize StatsUpperPeakEver; // Upper heap peak since startup

// NOTE(saeb): Bytes as KiB below one MiB, MiB above: "12.5 KiB", "1.13 MiB".
static void SandboxAppendBytes(String8* line, usize bytes)
{
    if(bytes < 1024 * 1024)
    {
        String8AppendReal(line, (real64)bytes / 1024.0, 1);
        String8Append(line, SV8(u8" KiB"));
    }
    else
    {
        String8AppendReal(line, (real64)bytes / (1024.0 * 1024.0), 2);
        String8Append(line, SV8(u8" MiB"));
    }
}

// NOTE(saeb): Shows the line as a label, then empties it for the next one.
static void SandboxStatsLine(String8* line)
{
    UILabel(&DebugUIContext, StringView8{ line->Data, line->Length });
    line->Length = 0;
}
#endif

void GameConfigure()
{
    WindowSetFlags(WindowFlags_None);
    WindowSetTitle(SV8(u8"Afterglow Sandbox"));
    WindowSetClientAreaDimensions(1280, 720);
    WindowSetMinClientAreaDimensions(360, 360);
    InputSetFlags(InputFlags_Mouse | InputFlags_Keyboard);
    RendererSetFlags(RendererFlags_VSync);
    RendererSetDesignSize(1000.0f, 1000.0f);
}

bool GameInit(StackAllocator* allocator)
{
#if SSTL_DEBUG
    AssetLoadFont(allocator, SV8(u8"Data/Engine/LiberationMono-Regular.aga"), &DebugFont);
#endif

    return(true);
}

void GameUpdate(StackAllocator* allocator, real64 deltaTime)
{
#if SSTL_DEBUG
    // Gather this frame; when the window is full, publish its numbers and start a new one.
    StatsElapsed += deltaTime;
    StatsFrames += 1;
    if(deltaTime > StatsWorstDelta)
    {
        StatsWorstDelta = deltaTime;
    }

    if(StatsElapsed >= SANDBOX_STATS_WINDOW)
    {
        StatsAverageMs = StatsElapsed / (real64)StatsFrames * 1000.0;
        StatsWorstMs = StatsWorstDelta * 1000.0;
        StatsFPS = (real64)StatsFrames / StatsElapsed;

        // NOTE(saeb): The peak so far also covers startup (asset loading), so it's kept as the "ever" peak; resetting then makes the next window's peak cover only that window.
        StatsUpperPeak = GetHeapPeakMemory(allocator, Heap::Upper);
        if(StatsUpperPeak > StatsUpperPeakEver)
        {
            StatsUpperPeakEver = StatsUpperPeak;
        }

        ResetPeakMemory(allocator);

        StatsElapsed = 0.0;
        StatsWorstDelta = 0.0;
        StatsFrames = 0;
    }

    RendererStats renderer;
    RendererGetStats(&renderer);

    usize lowerUsed = GetHeapUsedMemory(allocator, Heap::Lower);
    usize capacity = GetUsedMemory(allocator) + GetAvailableMemory(allocator);

    UIBegin(&DebugUIContext, &DebugFont);
    UIPanelBegin(&DebugUIContext, &DebugPanel, SV8(u8"Stats"));

    char8 buffer[96];
    String8 line = { buffer, 0, sizeof(buffer) };

    String8Append(&line, SV8(u8"Frame: "));
    String8AppendReal(&line, StatsAverageMs, 2);
    String8Append(&line, SV8(u8" ms ("));
    String8AppendReal(&line, StatsFPS, 0);
    String8Append(&line, SV8(u8" FPS)"));
    SandboxStatsLine(&line);

    String8Append(&line, SV8(u8"Worst frame: "));
    String8AppendReal(&line, StatsWorstMs, 2);
    String8Append(&line, SV8(u8" ms"));
    SandboxStatsLine(&line);

    String8Append(&line, SV8(u8"Draw calls: "));
    String8AppendUInt(&line, renderer.DrawCalls);
    SandboxStatsLine(&line);

    String8Append(&line, SV8(u8"Quads: "));
    String8AppendUInt(&line, renderer.Quads);
    String8Append(&line, SV8(u8" / "));
    String8AppendUInt(&line, renderer.MaxQuads);
    SandboxStatsLine(&line);

    String8Append(&line, SV8(u8"Dropped quads: "));
    String8AppendUInt(&line, renderer.DroppedQuads);
    SandboxStatsLine(&line);

    String8Append(&line, SV8(u8"Textures: "));
    String8AppendUInt(&line, renderer.Textures);
    String8Append(&line, SV8(u8" / "));
    String8AppendUInt(&line, renderer.MaxTextures);
    String8Append(&line, SV8(u8", pipelines: "));
    String8AppendUInt(&line, renderer.Pipelines);
    String8Append(&line, SV8(u8" / "));
    String8AppendUInt(&line, renderer.MaxPipelines);
    SandboxStatsLine(&line);

    String8Append(&line, SV8(u8"Lower heap: "));
    SandboxAppendBytes(&line, lowerUsed);
    SandboxStatsLine(&line);

    String8Append(&line, SV8(u8"Upper peak: "));
    SandboxAppendBytes(&line, StatsUpperPeak);
    String8Append(&line, SV8(u8" (ever "));
    SandboxAppendBytes(&line, StatsUpperPeakEver);
    String8Append(&line, SV8(u8")"));
    SandboxStatsLine(&line);

    String8Append(&line, SV8(u8"Memory: "));
    SandboxAppendBytes(&line, capacity);
    SandboxStatsLine(&line);

    UIPanelEnd(&DebugUIContext);

    UIEnd(&DebugUIContext);
#endif
}

void GameShutdown(StackAllocator* allocator)
{
}
