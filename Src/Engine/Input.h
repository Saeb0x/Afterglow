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

enum class MouseButton : uint8
{
    Unknown,

    Right,
    Wheel,
    Left,

    Count
};

struct ButtonState
{
    bool32 IsDown; // Currently held; persists across frames
    bool32 WasDown; // Held last frame; snapshotted at frame start
};

inline bool8 IsDown(ButtonState button)
{
    return(button.IsDown);
}

inline bool8 Pressed(ButtonState button)
{
    return(button.IsDown && !button.WasDown);
}

inline bool8 Released(ButtonState button)
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
    ButtonState Buttons[static_cast<usize>(MouseButton::Count)];
};

struct GameInput
{
    Keyboard Keyboard;
    Mouse Mouse;

    char TypedCharacters[32]; // This frame's WM_CHAR queue
    uint32 TypedCharacterCount;
};

inline ButtonState GetButtonState(GameInput* input, Key key)
{
    return(input->Keyboard.Keys[static_cast<usize>(key)]);
}

inline ButtonState GetButtonState(GameInput* input, MouseButton mouseButton)
{
    return(input->Mouse.Buttons[static_cast<usize>(mouseButton)]);
}

inline bool8 KeyDown(GameInput* input, Key key)
{
    return(IsDown(GetButtonState(input, key)));
}

inline bool8 KeyPressed(GameInput* input, Key key)
{
    return(Pressed(GetButtonState(input, key)));
}

inline bool8 KeyReleased(GameInput* input, Key key)
{
    return(Released(GetButtonState(input, key)));
}

inline bool8 MouseButtonDown(GameInput* input, MouseButton mouseButton)
{
    return(IsDown(GetButtonState(input, mouseButton)));
}

inline bool8 MouseButtonPressed(GameInput* input, MouseButton mouseButton)
{
    return(Pressed(GetButtonState(input, mouseButton)));
}

inline bool8 MouseButtonReleased(GameInput* input, MouseButton mouseButton)
{
    return(Released(GetButtonState(input, mouseButton)));
}

inline void InputBegin(GameInput* input)
{
    for(usize keyIndex = 0; keyIndex < static_cast<usize>(Key::Count); ++keyIndex)
    {
        input->Keyboard.Keys[keyIndex].WasDown = input->Keyboard.Keys[keyIndex].IsDown;
    }

    for(usize mouseButtonIndex = 0; mouseButtonIndex < static_cast<usize>(MouseButton::Count); ++mouseButtonIndex)
    {
        input->Mouse.Buttons[mouseButtonIndex].WasDown = input->Mouse.Buttons[mouseButtonIndex].IsDown;
    }

    input->TypedCharacterCount = 0;
}

#endif
