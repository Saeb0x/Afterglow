#include "Win32Input.h"

static Key Win32TranslateKey(WPARAM vkCode)
{
    switch(vkCode)
    {
        case VK_RETURN:
        {
            return(Key::KEY_ENTER);
        }

        case VK_BACK:
        {
            return(Key::KEY_BACKSPACE);
        }

        case VK_ESCAPE:
        {
            return(Key::KEY_ESCAPE);
        }

        case VK_OEM_3:
        {
            return(Key::KEY_TILDE); // US ANSI keyboard
        }

        case VK_TAB:
        {
            return(Key::KEY_TAB);
        }

        case VK_UP:
        {
            return(Key::KEY_UP);
        }

        case VK_DOWN:
        {
            return(Key::KEY_DOWN);
        }

        case VK_LEFT:
        {
            return(Key::KEY_LEFT);
        }

        case VK_RIGHT:
        {
            return(Key::KEY_RIGHT);
        }

        default:
        {
            return(Key::KEY_UNKNOWN);
        }
    }
}

void Win32BeginInputFrame(GameInput* input)
{
    for(usize keyIndex = 0; keyIndex < static_cast<usize>(Key::KEY_COUNT); ++keyIndex)
    {
        input->Keyboard.Keys[keyIndex].WasDown = input->Keyboard.Keys[keyIndex].IsDown;
    }

    input->Mouse.Left.WasDown = input->Mouse.Left.IsDown;
    input->Mouse.Right.WasDown = input->Mouse.Right.IsDown;
    input->Mouse.Middle.WasDown = input->Mouse.Middle.IsDown;

    input->TypedCharacterCount = 0;
}

void Win32ProcessInputMessage(GameInput* input, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        {
            Key key = Win32TranslateKey(wParam);
            if(key != Key::KEY_UNKNOWN)
            {
                input->Keyboard.Keys[static_cast<usize>(key)].IsDown = true;
            }
        } break;

        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            Key key = Win32TranslateKey(wParam);
            if(key != Key::KEY_UNKNOWN)
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
            input->Mouse.Left.IsDown = true;
        } break;

        case WM_LBUTTONUP:
        {
            input->Mouse.Left.IsDown = false;
        } break;

        case WM_RBUTTONDOWN:
        {
            input->Mouse.Right.IsDown = true;
        } break;

        case WM_RBUTTONUP:
        {
            input->Mouse.Right.IsDown = false;
        } break;

        case WM_MBUTTONDOWN:
        {
            input->Mouse.Middle.IsDown = true;
        } break;

        case WM_MBUTTONUP:
        {
            input->Mouse.Middle.IsDown = false;
        } break;

        case WM_KILLFOCUS:
        {
            // NOTE(saeb): Lost focus mid-press; the KEY_UP goes to another window, so clear everything to avoid stuck keys.
            for(usize keyIndex = 0; keyIndex < static_cast<usize>(Key::KEY_COUNT); ++keyIndex)
            {
                input->Keyboard.Keys[keyIndex].IsDown = false;
            }

            input->Mouse.Left.IsDown = false;
            input->Mouse.Right.IsDown = false;
            input->Mouse.Middle.IsDown = false;
        } break;
    }
}
