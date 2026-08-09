#if !defined(AFTERGLOW_INPUT_H)
#define AFTERGLOW_INPUT_H

#include "Engine/Types.h"

enum class Key : uint8
{
    KEY_UNKNOWN,

    KEY_ENTER, KEY_BACKSPACE, KEY_ESCAPE, KEY_TILDE, KEY_TAB,
    KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT,

    KEY_COUNT
};

struct ButtonState
{
    bool32 IsDown; // Currently held; persists across frames
    bool32 WasDown; // Held last frame; snapshotted at frame start
};

struct Keyboard
{
    ButtonState Keys[static_cast<usize>(Key::KEY_COUNT)];
};

struct Mouse
{
    int32 X;
    int32 Y;
    ButtonState Right;
    ButtonState Middle;
    ButtonState Left;
};

struct GameInput
{
    Keyboard Keyboard;
    Mouse Mouse;

    char TypedCharacters[32]; // This frame's WM_CHAR queue
    uint32 TypedCharacterCount;
};

inline bool32 IsDown(ButtonState button)
{
    return(button.IsDown);
}

inline bool32 Pressed(ButtonState button)
{
    return(button.IsDown && !button.WasDown);
}

inline bool32 Released(ButtonState button)
{
    return(!button.IsDown && button.WasDown);
}

#endif
