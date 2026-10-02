#if !defined(AFTERGLOW_UI_H)
#define AFTERGLOW_UI_H

#include <SSTL/Core/Types.h>
#include <SSTL/Core/String.h>

#include "Engine/Renderer/Text.h"

using UIID = uint32; // 0 = no widget

struct UIContext
{
    // Input for this frame, in screen pixels; copied in UIBegin.
    real32 MouseX, MouseY;
    bool MouseDown, MousePressed, MouseReleased;

    UIID Hot; // Under the mouse, decided last frame
    UIID NextHot; // Being decided this frame; the last widget drawn under the mouse wins
    UIID Active; // The mouse went down on it and hasn't been released yet; persists across frames

    const Font* Font;

    real32 DragOffsetX, DragOffsetY; // Where the active widget was grabbed, relative to the corner it moves (top-left for the title bar, bottom-right for the grip)
    real32 ScreenWidth, ScreenHeight; // Client area size, so panels can stay inside it

    // NOTE(saeb): Set by UIPanelBegin; widgets inside the panel read them.
    UIID Seed; // The current panel's ID; widget IDs are derived from it. 0 outside panels
    real32 Scale; // Panel size relative to its base size
    real32 LayoutX, LayoutY, LayoutWidth; // Where the next row goes, and how wide rows are
};

enum UIPanelFlags : uint32
{
    UIPanelFlags_None = 0,
    UIPanelFlags_NoTitle = 1 << 0,
    UIPanelFlags_NoMove = 1 << 1,
    UIPanelFlags_NoResize = 1 << 2
};

struct UIPanel
{
    real32 X, Y, Width, Height; // Screen pixels; the game sets the start, dragging and resizing change them
    real32 BaseWidth, BaseHeight; // The size its contents are designed for; at this size they draw at scale 1
    uint32 Flags;
};

void UIBegin(UIContext* ui, const Font* font);
void UIEnd(UIContext* ui);

void UIPanelBegin(UIContext* ui, UIPanel* panel, StringView8 title);
void UIPanelEnd(UIContext* ui);

// NOTE(saeb): True while the mouse is over any UI or dragging one; the game should ignore mouse clicks then. Reflects the last UIEnd.
bool UIWantsMouse(const UIContext* ui);

// NOTE(saeb): Every widget below takes the next row of the current panel. Labels double as IDs, so two widgets in one panel need different labels.

// Returns true on the frame it's clicked.
bool UIButton(UIContext* ui, StringView8 label);

// Plain text, left-aligned. Not interactive, so it needs no ID and any text can repeat.
void UILabel(UIContext* ui, StringView8 text);

// Clicking anywhere on the row flips *value. Returns true on the frame it flips.
bool UICheckbox(UIContext* ui, StringView8 label, bool* value);

// Press and drag along the track to set *value between minimum and maximum; shows "label: value". Returns true on frames the value changes.
bool UISlider(UIContext* ui, StringView8 label, real32* value, real32 minimum, real32 maximum);

#endif
