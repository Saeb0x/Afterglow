#include "Win32Input.h"
#include "Engine/Input.h"

#include <SSTL/Utility.h>

struct InputButtonState
{
    bool32 IsDown; // Currently held; persists across frames
    bool32 WasDown; // Held last frame; snapshotted at frame start
};

static bool8 InputIsDown(InputButtonState button)
{
    return(button.IsDown);
}

static bool8 InputPressed(InputButtonState button)
{
    return(button.IsDown && !button.WasDown);
}

static bool8 InputReleased(InputButtonState button)
{
    return(!button.IsDown && button.WasDown);
}

struct InputKeyboard
{
    InputButtonState Keys[static_cast<usize>(InputKey::Count)];
};

struct InputMouse
{
    int32 X;
    int32 Y;
    InputButtonState Buttons[static_cast<usize>(InputMouseButton::Count)];
};

struct Input
{
    InputKeyboard Keyboard;
    InputMouse Mouse;

    char8 TypedCharacters[32]; // This frame's WM_CHAR queue
    uint32 TypedCharacterCount;
};
static Input InputData = {};

static InputButtonState InputGetButtonState(InputKey key)
{
    return(InputData.Keyboard.Keys[static_cast<usize>(key)]);
}

static InputButtonState InputGetButtonState(InputMouseButton mouseButton)
{
    return(InputData.Mouse.Buttons[static_cast<usize>(mouseButton)]);
}

bool8 InputKeyDown(InputKey key)
{
    return(InputIsDown(InputGetButtonState(key)));
}

bool8 InputKeyPressed(InputKey key)
{
    return(InputPressed(InputGetButtonState(key)));
}

bool8 InputKeyReleased(InputKey key)
{
    return(InputReleased(InputGetButtonState(key)));
}

bool8 InputMouseButtonDown(InputMouseButton mouseButton)
{
    return(InputIsDown(InputGetButtonState(mouseButton)));
}

bool8 InputMouseButtonPressed(InputMouseButton mouseButton)
{
    return(InputPressed(InputGetButtonState(mouseButton)));
}

bool8 InputMouseButtonReleased(InputMouseButton mouseButton)
{
    return(InputReleased(InputGetButtonState(mouseButton)));
}

void InputMouseXY(int32* x, int32* y)
{
    *x = InputData.Mouse.X;
    *y = InputData.Mouse.Y;
}

static InputKey Win32InputTranslateKey(WPARAM virtualKeyCode)
{
    switch(virtualKeyCode)
    {
        case VK_BACK:
        {
            return(InputKey::Backspace);
        }

        case VK_RETURN:
        {
            return(InputKey::Enter);
        }

        case VK_ESCAPE:
        {
            return(InputKey::Escape);
        }

        case VK_OEM_3:
        {
            return(InputKey::Tilde); // US ANSI keyboard
        }

        case VK_TAB:
        {
            return(InputKey::Tab);
        }

        case VK_RIGHT:
        {
            return(InputKey::Right);
        }

        case VK_UP:
        {
            return(InputKey::Up);
        }

        case VK_DOWN:
        {
            return(InputKey::Down);
        }

        case VK_LEFT:
        {
            return(InputKey::Left);
        }

        default:
        {
            return(InputKey::Unknown);
        }
    }
}

void Win32InputBegin()
{
    for(usize keyIndex = 0; keyIndex < static_cast<usize>(InputKey::Count); ++keyIndex)
    {
        InputData.Keyboard.Keys[keyIndex].WasDown = InputData.Keyboard.Keys[keyIndex].IsDown;
    }

    for(usize mouseButtonIndex = 0; mouseButtonIndex < static_cast<usize>(InputMouseButton::Count); ++mouseButtonIndex)
    {
        InputData.Mouse.Buttons[mouseButtonIndex].WasDown = InputData.Mouse.Buttons[mouseButtonIndex].IsDown;
    }

    InputData.TypedCharacterCount = 0;
}

void Win32InputProcess(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        {
            InputKey key = Win32InputTranslateKey(wParam);
            if(key != InputKey::Unknown)
            {
                InputData.Keyboard.Keys[static_cast<usize>(key)].IsDown = true;
            }
        } break;

        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            InputKey key = Win32InputTranslateKey(wParam);
            if(key != InputKey::Unknown)
            {
                InputData.Keyboard.Keys[static_cast<usize>(key)].IsDown = false;
            }
        } break;

        case WM_CHAR:
        {
            uint32 character = (uint32)wParam;
            if(character >= 32 && character < 127) // Printable ASCII
            {
                if(InputData.TypedCharacterCount < sstl::ArrayCount(InputData.TypedCharacters))
                {
                    InputData.TypedCharacters[InputData.TypedCharacterCount++] = (char)character;
                }
            }
        } break;

        case WM_MOUSEMOVE:
        {
            InputData.Mouse.X = (int32)(int16)LOWORD(lParam);
            InputData.Mouse.Y = (int32)(int16)HIWORD(lParam);
        } break;

        case WM_LBUTTONDOWN:
        {
            InputData.Mouse.Buttons[static_cast<usize>(InputMouseButton::Left)].IsDown = true;
        } break;

        case WM_LBUTTONUP:
        {
            InputData.Mouse.Buttons[static_cast<usize>(InputMouseButton::Left)].IsDown = false;
        } break;

        case WM_RBUTTONDOWN:
        {
            InputData.Mouse.Buttons[static_cast<usize>(InputMouseButton::Right)].IsDown = true;
        } break;

        case WM_RBUTTONUP:
        {
            InputData.Mouse.Buttons[static_cast<usize>(InputMouseButton::Right)].IsDown = false;
        } break;

        case WM_MBUTTONDOWN:
        {
            InputData.Mouse.Buttons[static_cast<usize>(InputMouseButton::Middle)].IsDown = true;
        } break;

        case WM_MBUTTONUP:
        {
            InputData.Mouse.Buttons[static_cast<usize>(InputMouseButton::Middle)].IsDown = false;
        } break;

        case WM_KILLFOCUS:
        {
            // NOTE(saeb): Lost focus mid-press; the KEY_UP goes to another window, so clear everything to avoid stuck keys.
            for(usize keyIndex = 0; keyIndex < static_cast<usize>(InputKey::Count); ++keyIndex)
            {
                InputData.Keyboard.Keys[keyIndex].IsDown = false;
            }

            for(usize mouseButtonIndex = 0; mouseButtonIndex < static_cast<usize>(InputMouseButton::Count); ++mouseButtonIndex)
            {
                InputData.Mouse.Buttons[mouseButtonIndex].IsDown = false;
            }
        } break;
    }
}
