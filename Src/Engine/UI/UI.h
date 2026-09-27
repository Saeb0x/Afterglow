#if !defined(AFTERGLOW_UI_H)
#define AFTERGLOW_UI_H

#include <SSTL/Core/Types.h>
#include <SSTL/Core/String.h>

#include "Engine/Renderer/Text.h"

using UIID = uint32; // 0 = no widget

struct UIContext
{
    // Input for this frame, in window pixels; copied in UIBegin.
    real32 MouseX, MouseY;
    bool MouseDown, MousePressed, MouseReleased;

    UIID Hot; // Under the mouse, decided last frame
    UIID NextHot; // Being decided this frame; the last widget drawn under the mouse wins
    UIID Active; // The mouse went down on it and hasn't been released yet; persists across frames

    const Font* Font;
};

void UIBegin(UIContext* ui, const Font* font);
void UIEnd(UIContext* ui);

bool UIButton(UIContext* ui, StringView8 label, real32 x, real32 y, real32 width, real32 height);

#endif
