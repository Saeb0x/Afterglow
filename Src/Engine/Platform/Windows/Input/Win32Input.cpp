#include "Win32Input.h"

static Key Win32TranslateKey(WPARAM vkCode)
{
    switch(vkCode)
    {
        case VK_RETURN:
        {
            return(KEY_ENTER);
        }

        case VK_ESCAPE:
        {
            return(KEY_ESCAPE);
        }

        case VK_BACK:
        {
            return(KEY_BACKSPACE);
        }

        case VK_TAB:
        {
            return(KEY_TAB);
        }

        case VK_UP:
        {
            return(KEY_UP);
        }

        case VK_DOWN:
        {
            return(KEY_DOWN);
        }

        case VK_LEFT:
        {
            return(KEY_LEFT);
        }

        case VK_RIGHT:
        {
            return(KEY_RIGHT);
        }

        case VK_OEM_3:
        {
            return(KEY_TILDE); // US ANSI keyboard
        }

        default:
        {
            return(KEY_UNKNOWN);
        }
    }
}

void Win32BeginInputFrame(GameInput* input)
{
    for(int32 keyIndex = 0; keyIndex < KEY_COUNT; ++keyIndex)
    {
        input->Keys[keyIndex].WasDown = input->Keys[keyIndex].IsDown;
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
            if(key != KEY_UNKNOWN)
            {
                input->Keys[key].IsDown = true;
            }
        } break;

        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            Key key = Win32TranslateKey(wParam);
            if(key != KEY_UNKNOWN)
            {
                input->Keys[key].IsDown = false;
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
            for(int32 keyIndex = 0; keyIndex < KEY_COUNT; ++keyIndex)
            {
                input->Keys[keyIndex].IsDown = false;
            }

            input->Mouse.Left.IsDown = false;
            input->Mouse.Right.IsDown = false;
            input->Mouse.Middle.IsDown = false;
        } break;
    }
}
