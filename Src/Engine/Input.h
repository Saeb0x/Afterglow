#if !defined(AFTERGLOW_INPUT_H)
#define AFTERGLOW_INPUT_H

#include "Engine/Types.h"

enum class InputKey : uint8
{
    Unknown,

    Backspace, Enter, Escape, Tilde, Tab,
    Right, Up, Down, Left,

    Count
};

enum class InputMouseButton : uint8
{
    Unknown,

    Right,
    Middle,
    Left,

    Count
};

bool8 InputKeyDown(InputKey key);
bool8 InputKeyPressed(InputKey key);
bool8 InputKeyReleased(InputKey key);

bool8 InputMouseButtonDown(InputMouseButton mouseButton);
bool8 InputMouseButtonPressed(InputMouseButton mouseButton);
bool8 InputMouseButtonReleased(InputMouseButton mouseButton);
void InputMouseXY(int32* x, int32* y);

#endif
