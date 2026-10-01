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
void WindowSetTitle(StringView8 title);
void WindowSetClientAreaDimensions(int32 width, int32 height);
void WindowSetMinClientAreaDimensions(int32 width, int32 height);
void WindowGetClientAreaDimensions(int32* width, int32* height);
bool WindowGetMinimized();
void WindowRequestClose();
void WindowSetCursorVisible(bool visible);

#endif
