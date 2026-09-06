#if !defined(AFTERGLOW_INPUT_H)
#define AFTERGLOW_INPUT_H

#include "Engine/Types.h"

enum InputFlags : uint32
{
    InputFlags_None = 0,
    InputFlags_Mouse = 1 << 0,
    InputFlags_Keyboard = 1 << 1
};

enum class InputKey : uint8
{
    Unknown,

    Backspace, Enter, Space, Escape, Tilde, Tab,
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

void InputSetFlags(uint32 inputFlags);

bool8 InputKeyDown(InputKey key);
bool8 InputKeyPressed(InputKey key);
bool8 InputKeyReleased(InputKey key);

bool8 InputMouseButtonDown(InputMouseButton mouseButton);
bool8 InputMouseButtonPressed(InputMouseButton mouseButton);
bool8 InputMouseButtonReleased(InputMouseButton mouseButton);
void InputMouseXY(int32* x, int32* y);

#endif
