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

    uint32 Flags;
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

        case VK_SPACE:
        {
            return(InputKey::Space);
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
    if(InputData.Flags & InputFlags_Keyboard)
    {
        for(usize keyIndex = 0; keyIndex < static_cast<usize>(InputKey::Count); ++keyIndex)
        {
            InputData.Keyboard.Keys[keyIndex].WasDown = InputData.Keyboard.Keys[keyIndex].IsDown;
        }

        InputData.TypedCharacterCount = 0;
    }

    if(InputData.Flags & InputFlags_Mouse)
    {
        for(usize mouseButtonIndex = 0; mouseButtonIndex < static_cast<usize>(InputMouseButton::Count); ++mouseButtonIndex)
        {
            InputData.Mouse.Buttons[mouseButtonIndex].WasDown = InputData.Mouse.Buttons[mouseButtonIndex].IsDown;
        }
    }
}

void Win32InputProcess(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_KEYUP:
        case WM_KEYDOWN:
        case WM_SYSKEYUP:
        case WM_SYSKEYDOWN:
        {
            if(!(InputData.Flags & InputFlags_Keyboard))
            {
                break;
            }

            InputKey key = Win32InputTranslateKey(wParam);

            if(key != InputKey::Unknown)
            {
                if(message == WM_KEYUP || message == WM_SYSKEYUP)
                {
                    InputData.Keyboard.Keys[static_cast<usize>(key)].IsDown = false;
                }
                else
                {
                    InputData.Keyboard.Keys[static_cast<usize>(key)].IsDown = true;
                }
            }
        } break;

        case WM_CHAR:
        {
            if(!(InputData.Flags & InputFlags_Keyboard))
            {
                break;
            }

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
            if(!(InputData.Flags & InputFlags_Mouse))
            {
                break;
            }

            InputData.Mouse.X = (int32)(int16)LOWORD(lParam);
            InputData.Mouse.Y = (int32)(int16)HIWORD(lParam);
        } break;

        case WM_RBUTTONUP:
        {
            if(InputData.Flags & InputFlags_Mouse)
            {
                InputData.Mouse.Buttons[static_cast<usize>(InputMouseButton::Right)].IsDown = false;
            }
        } break;

        case WM_RBUTTONDOWN:
        {
            if(InputData.Flags & InputFlags_Mouse)
            {
                InputData.Mouse.Buttons[static_cast<usize>(InputMouseButton::Right)].IsDown = true;
            }
        } break;

        case WM_MBUTTONUP:
        {
            if(InputData.Flags & InputFlags_Mouse)
            {
                InputData.Mouse.Buttons[static_cast<usize>(InputMouseButton::Middle)].IsDown = false;
            }
        } break;

        case WM_MBUTTONDOWN:
        {
            if(InputData.Flags & InputFlags_Mouse)
            {
                InputData.Mouse.Buttons[static_cast<usize>(InputMouseButton::Middle)].IsDown = true;
            }
        } break;

        case WM_LBUTTONUP:
        {
            if(InputData.Flags & InputFlags_Mouse)
            {
                InputData.Mouse.Buttons[static_cast<usize>(InputMouseButton::Left)].IsDown = false;
            }
        } break;

        case WM_LBUTTONDOWN:
        {
            if(InputData.Flags & InputFlags_Mouse)
            {
                InputData.Mouse.Buttons[static_cast<usize>(InputMouseButton::Left)].IsDown = true;
            }
        } break;

        case WM_KILLFOCUS:
        {
            // NOTE(saeb): Lost focus mid-press; the KEY_UP goes to another window, so clear everything to avoid stuck keys and buttons.
            if(InputData.Flags & InputFlags_Keyboard)
            {
                for(usize keyIndex = 0; keyIndex < static_cast<usize>(InputKey::Count); ++keyIndex)
                {
                    InputData.Keyboard.Keys[keyIndex].IsDown = false;
                }

            }

            if(InputData.Flags & InputFlags_Mouse)
            {
                for(usize mouseButtonIndex = 0; mouseButtonIndex < static_cast<usize>(InputMouseButton::Count); ++mouseButtonIndex)
                {
                    InputData.Mouse.Buttons[mouseButtonIndex].IsDown = false;
                }
            }
        } break;
    }
}

void InputSetFlags(uint32 inputFlags)
{
    InputData.Flags = inputFlags;
}

bool8 InputKeyDown(InputKey key)
{
    if(InputData.Flags & InputFlags_Keyboard)
    {
        return(InputIsDown(InputGetButtonState(key)));
    }
    else
    {
        return(false);
    }
}

bool8 InputKeyPressed(InputKey key)
{
    if(InputData.Flags & InputFlags_Keyboard)
    {
        return(InputPressed(InputGetButtonState(key)));
    }
    else
    {
        return(false);
    }
}

bool8 InputKeyReleased(InputKey key)
{
    if(InputData.Flags & InputFlags_Keyboard)
    {
        return(InputReleased(InputGetButtonState(key)));
    }
    else
    {
        return(false);
    }
}

bool8 InputMouseButtonDown(InputMouseButton mouseButton)
{
    if(InputData.Flags & InputFlags_Mouse)
    {
        return(InputIsDown(InputGetButtonState(mouseButton)));
    }
    else
    {
        return(false);
    }
}

bool8 InputMouseButtonPressed(InputMouseButton mouseButton)
{
    if(InputData.Flags & InputFlags_Mouse)
    {
        return(InputPressed(InputGetButtonState(mouseButton)));
    }
    else
    {
        return(false);
    }
}

bool8 InputMouseButtonReleased(InputMouseButton mouseButton)
{
    if(InputData.Flags & InputFlags_Mouse)
    {
        return(InputReleased(InputGetButtonState(mouseButton)));
    }
    else
    {
        return(false);
    }
}

void InputMouseXY(int32* x, int32* y)
{
    if(InputData.Flags & InputFlags_Mouse)
    {
        *x = InputData.Mouse.X;
        *y = InputData.Mouse.Y;
    }
    else
    {
        *x = 0;
        *y = 0;
    }
}
