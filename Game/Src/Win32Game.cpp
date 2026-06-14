#include "Afterglow.h"

#include <windows.h>

static bool Running;
static BITMAPINFO BitmapInfo;
static u32 BitmapWidth;
static u32 BitmapHeight;
static u8 BytesPerPixel = 4;
static void* BitmapMemory;

static void FillScreen()
{
    u32* pixel = (u32*)BitmapMemory;
    for(size_t i = 0; i < (BitmapWidth * BitmapHeight); i++)
    {
        /* NOTE(saeb): GDI walks those bytes in address order and reads them as B G R.
         *
         * (MSB) FF 00 00 FF (LSB)
         * Byte 0: 0xFF
         * Byte 1: 0x00
         * Byte 2: 0x00
         * Byte 3: 0xFF
         *
         * Little-endian flips this in memory (LSB goes to the lowest address).
         * Address+0: 0xFF (LSB)    -> BB
         * Address+1: 0x00          -> GG
         * Address+2: 0x00          -> RR
         * Address+3: 0xFF (MSB)    -> IGNORED
         */
        *pixel++ = 0xFF0000FF;
    }
}

static void ResizeDIB(RECT* windowRect)
{   
    BitmapWidth = windowRect->right - windowRect->left;
    BitmapHeight = windowRect->bottom - windowRect->top;
    
    if(BitmapWidth == 0 || BitmapHeight == 0)
        return;
    
    BitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    BitmapInfo.bmiHeader.biWidth = BitmapWidth;
    BitmapInfo.bmiHeader.biHeight = -(i32)BitmapHeight;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = BytesPerPixel * 8;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;

    u64 bitmapMemorySize = (BitmapWidth * BitmapHeight) * BytesPerPixel;

    if(BitmapMemory)
    {
        VirtualFree(BitmapMemory, 0, MEM_RELEASE);
    }
    
    BitmapMemory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

static void BlitWindowDC(HDC* deviceContext, RECT* windowRect)
{
    u32 windowWidth = windowRect->right - windowRect->left;
    u32 windowHeight = windowRect->bottom - windowRect->top;
    
    StretchDIBits(*deviceContext,
                  0, 0, windowWidth, windowHeight,
                  0, 0, BitmapWidth, BitmapHeight,
                  BitmapMemory,
                  &BitmapInfo,
                  DIB_RGB_COLORS,
                  SRCCOPY);
}

LRESULT CALLBACK WindowCallback(HWND handle,
                                UINT message,
                                WPARAM wParam,
                                LPARAM lParam)
{
    LRESULT result = 0;
    
    switch(message)
    {
        case WM_CLOSE:
        {
            // TODO(saeb): User chose to close - show confirmation UI?
            DestroyWindow(handle);
        } break;
        case WM_DESTROY:
        {
            // TODO(saeb): Window is being destroyed; perform shutdown cleanup.
            PostQuitMessage(0);
        } break;
        case WM_SIZE:
        {
            RECT windowRect;
            GetClientRect(handle, &windowRect);
            ResizeDIB(&windowRect);
        } break;
        case WM_PAINT:
        {
            PAINTSTRUCT paint;
            HDC deviceContext = BeginPaint(handle, &paint);
            RECT windowRect;
            GetClientRect(handle, &windowRect);
            FillScreen();
            BlitWindowDC(&deviceContext, &windowRect);
            EndPaint(handle, &paint);
        } break;
        default:
        {
            return DefWindowProc(handle, message, wParam, lParam);
        } break;
    }
    
    return(result);
}

int WINAPI WinMain(HINSTANCE instance,
                   HINSTANCE,
                   LPSTR,
                   int)
{
    WNDCLASSEX windowClass = {};
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_OWNDC;
    windowClass.lpfnWndProc = WindowCallback;
    windowClass.hInstance = instance;
    // windowClass.hIcon = ;
    // windowClass.hCursor = ;
    windowClass.lpszClassName = "AfterglowWindowClass";
    // windowClass.hIconSm = ;
    
    if(RegisterClassEx(&windowClass))
    {
        HWND windowHandle = CreateWindowEx(0,
                                           windowClass.lpszClassName,
                                           "Afterglow Game",
                                           WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                           CW_USEDEFAULT, CW_USEDEFAULT,
                                           CW_USEDEFAULT, CW_USEDEFAULT,
                                           0,
                                           0,
                                           windowClass.hInstance,
                                           0);
        
        if(windowHandle)
        {
            Running = true;
            while(Running)
            {
                MSG message;
                while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
                {
                    if(message.message == WM_QUIT)
                    {
                        Running = false;
                        break;
                    }
                    
                    TranslateMessage(&message);
                    DispatchMessage(&message);
                }
            }
        }
        else
        {
            // TODO(saeb): Logging.
        }                       
    }
    else
    {
        // TODO(saeb): Logging.
    }
    
    return(0);
}
