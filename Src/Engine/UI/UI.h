#if !defined(AFTERGLOW_UI_H)
#define AFTERGLOW_UI_H

#include <SSTL/Core/Types.h>
#include <SSTL/Core/String.h>

#include "Engine/Renderer/Text.h"

using UIID = uint32; // 0 = no widget

#define UI_TITLE_HEIGHT 30.0f

struct UIContext
{
    // Input for this frame, in window pixels; copied in UIBegin.
    real32 MouseX, MouseY;
    bool MouseDown, MousePressed, MouseReleased;

    UIID Hot; // Under the mouse, decided last frame
    UIID NextHot; // Being decided this frame; the last widget drawn under the mouse wins
    UIID Active; // The mouse went down on it and hasn't been released yet; persists across frames

    const Font* Font;

    real32 DragOffsetX, DragOffsetY; // Where the title bar was grabbed, relative to the panel's corner
    real32 ScreenWidth, ScreenHeight; // Client area size, so panels can stay inside it
};

struct UIPanel
{
    real32 X, Y, Width, Height; // Window pixels; the game sets the start, dragging changes X and Y
};

void UIBegin(UIContext* ui, const Font* font);
void UIEnd(UIContext* ui);

void UIPanelBegin(UIContext* ui, UIPanel* panel, StringView8 title);
void UIPanelEnd(UIContext* ui);

bool UIButton(UIContext* ui, StringView8 label, real32 x, real32 y, real32 width, real32 height);

#endif
