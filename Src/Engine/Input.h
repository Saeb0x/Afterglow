#if !defined(AFTERGLOW_INPUT_H)
#define AFTERGLOW_INPUT_H

#include "Engine/Types.h"

enum Key
{
    KEY_UNKNOWN,

    KEY_ENTER, KEY_ESCAPE, KEY_BACKSPACE, KEY_TAB,
    KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT,
    KEY_TILDE,

    KEY_COUNT
};

struct ButtonState
{
    bool32 IsDown; // Currently held; persists across frames
    bool32 WasDown; // Held last frame; snapshotted at frame start
};

struct MouseState
{
    int32 X;
    int32 Y;
    ButtonState Right;
    ButtonState Middle;
    ButtonState Left;
};

struct GameInput
{
    ButtonState Keys[KEY_COUNT];
    MouseState Mouse;

    char TypedCharacters[32]; // This frame's WM_CHAR queue
    uint32 TypedCharacterCount;
};

static bool32 IsDown(ButtonState button)
{
    return(button.IsDown);
}

static bool32 Pressed(ButtonState button)
{
    return(button.IsDown && !button.WasDown);
}

static bool32 Released(ButtonState button)
{
    return(!button.IsDown && button.WasDown);
}

#endif
