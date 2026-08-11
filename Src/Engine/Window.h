#if !defined(AFTERGLOW_WINDOW_H)
#define AFTERGLOW_WINDOW_H

#include "Engine/Types.h"

struct GameInput;

struct WindowDimensions
{
    uint32 Width;
    uint32 Height;
};

struct Window
{
    void* NativeHandler;

    const char* Title;
    WindowDimensions Dimensions;
    bool32 Minimized;

    bool32 ShouldClose;
};

Window* WindowCreate(const char* title, uint32 width, uint32 height);
void WindowShow(Window* window);
void WindowShutdown(Window* window);

void WindowPumpEvents(Window* window, GameInput* input);

bool8 WindowShouldClose(Window* window);
void WindowGetDimensions(Window* window, WindowDimensions* outDims);
bool8 WindowGetMinimized(Window* window);

#endif
