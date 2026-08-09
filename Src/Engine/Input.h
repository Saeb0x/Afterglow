#if !defined(AFTERGLOW_INPUT_H)
#define AFTERGLOW_INPUT_H

#include "Engine/Types.h"

enum class Key : uint8
{
    Unknown,

    Enter, Backspace, Escape, Tilde, Tab,
    Up, Down, Left, Right,

    Count
};

struct ButtonState
{
    bool32 IsDown; // Currently held; persists across frames
    bool32 WasDown; // Held last frame; snapshotted at frame start
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

struct Keyboard
{
    ButtonState Keys[static_cast<usize>(Key::Count)];
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

inline ButtonState GetKeyButtonState(GameInput* input, Key key)
{
    return(input->Keyboard.Keys[static_cast<usize>(key)]);
}

inline bool32 KeyDown(GameInput* input, Key key)
{
    return(IsDown(GetKeyButtonState(input, key)));
}

inline bool32 KeyPressed(GameInput* input, Key key)
{
    return(Pressed(GetKeyButtonState(input, key)));
}

inline bool32 KeyReleased(GameInput* input, Key key)
{
    return(Released(GetKeyButtonState(input, key)));
}

#endif
