#include "Win32Input.h"

#include <SSTL/Utility.h>

static Key Win32KeyTranslate(WPARAM vkCode)
{
    switch(vkCode)
    {
        case VK_RETURN:
        {
            return(Key::Enter);
        }

        case VK_BACK:
        {
            return(Key::Backspace);
        }

        case VK_ESCAPE:
        {
            return(Key::Escape);
        }

        case VK_OEM_3:
        {
            return(Key::Tilde); // US ANSI keyboard
        }

        case VK_TAB:
        {
            return(Key::Tab);
        }

        case VK_UP:
        {
            return(Key::Up);
        }

        case VK_DOWN:
        {
            return(Key::Down);
        }

        case VK_LEFT:
        {
            return(Key::Left);
        }

        case VK_RIGHT:
        {
            return(Key::Right);
        }

        default:
        {
            return(Key::Unknown);
        }
    }
}

void Win32InputProcess(GameInput* input, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        {
            Key key = Win32KeyTranslate(wParam);
            if(key != Key::Unknown)
            {
                input->Keyboard.Keys[static_cast<usize>(key)].IsDown = true;
            }
        } break;

        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            Key key = Win32KeyTranslate(wParam);
            if(key != Key::Unknown)
            {
                input->Keyboard.Keys[static_cast<usize>(key)].IsDown = false;
            }
        } break;

        case WM_CHAR:
        {
            uint32 character = (uint32)wParam;
            if(character >= 32 && character < 127) // Printable ASCII; control keys handled via key state
            {
                if(input->TypedCharacterCount < sstl::ArrayCount(input->TypedCharacters))
                {
                    input->TypedCharacters[input->TypedCharacterCount++] = (char)character;
                }
            }
        } break;

        case WM_MOUSEMOVE:
        {
            input->Mouse.X = (int32)(int16)LOWORD(lParam);
            input->Mouse.Y = (int32)(int16)HIWORD(lParam);
        } break;

        case WM_LBUTTONDOWN:
        {
            input->Mouse.Buttons[static_cast<usize>(MouseButton::Left)].IsDown = true;
        } break;

        case WM_LBUTTONUP:
        {
            input->Mouse.Buttons[static_cast<usize>(MouseButton::Left)].IsDown = false;
        } break;

        case WM_RBUTTONDOWN:
        {
            input->Mouse.Buttons[static_cast<usize>(MouseButton::Right)].IsDown = true;
        } break;

        case WM_RBUTTONUP:
        {
            input->Mouse.Buttons[static_cast<usize>(MouseButton::Right)].IsDown = false;
        } break;

        case WM_MBUTTONDOWN:
        {
            input->Mouse.Buttons[static_cast<usize>(MouseButton::Wheel)].IsDown = true;
        } break;

        case WM_MBUTTONUP:
        {
            input->Mouse.Buttons[static_cast<usize>(MouseButton::Wheel)].IsDown = false;
        } break;

        case WM_KILLFOCUS:
        {
            // NOTE(saeb): Lost focus mid-press; the KEY_UP goes to another window, so clear everything to avoid stuck keys.
            for(usize keyIndex = 0; keyIndex < static_cast<usize>(Key::Count); ++keyIndex)
            {
                input->Keyboard.Keys[keyIndex].IsDown = false;
            }

            for(usize mouseButtonIndex = 0; mouseButtonIndex < static_cast<usize>(MouseButton::Count); ++mouseButtonIndex)
            {
                input->Mouse.Buttons[mouseButtonIndex].IsDown = false;
            }
        } break;
    }
}
