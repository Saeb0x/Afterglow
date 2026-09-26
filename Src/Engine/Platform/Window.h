#if !defined(AFTERGLOW_WINDOW_H)
#define AFTERGLOW_WINDOW_H

#include <SSTL/Core/Types.h>
#include <SSTL/Core/String.h>

enum WindowFlags : uint32
{
    WindowFlags_None = 0,
    WindowFlags_Fullscreen = 1 << 0
};

void WindowSetFlags(uint32 windowFlags);
void WindowSetTitle(StringView8 title); // The default is "Afterglow"
void WindowSetClientAreaDimensions(uint32 width, uint32 height); // The default is 1280 x 720
void WindowSetMinClientAreaDimensions(uint32 width, uint32 height); // The smallest client area the user can resize the window to; 0 means no limit on that side. Call it in GameConfigure, like WindowSetFlags.
void WindowGetClientAreaDimensions(uint32* width, uint32* height);
bool WindowGetMinimized();

#endif
