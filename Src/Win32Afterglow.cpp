#include "Core/Types.h"

#include <windows.h>

static bool Running;
static int32 XOffset;

struct BitmapBuffer
{
    BITMAPINFO Info;
    void* Data;
    
    int32 Width;
    int32  Height;
    
    uint8 BytesPerPixel;
    int32 Pitch;
};
static BitmapBuffer Buffer;

struct WindowDimensions
{
    int32 Width;
    int32 Height;
};

static WindowDimensions GetWindowDimensions(HWND windowHandle)
{
    RECT windowClientRect;
    GetClientRect(windowHandle, &windowClientRect);
    
    WindowDimensions windowDims;
    windowDims.Width = windowClientRect.right - windowClientRect.left;
    windowDims.Height = windowClientRect.bottom - windowClientRect.top;
    
    return windowDims;
}

static void FillScreen(BitmapBuffer* buffer)
{
    uint8* row = (uint8*)buffer->Data;
    for(int32 y = 0; y < buffer->Height; ++y)
    {
        uint32* pixel = (uint32*)row;
        
        for(int32 x = 0; x < buffer->Width; ++x)
        {
            uint8 blue = (uint8)(x + XOffset);
            uint8 green = 0;
            uint8 red = (uint8)y;

            *pixel++ = (red << 16) | (green << 8) | blue;  
        }

        row += buffer->Pitch;
    }
}

static void SetupBitmapBuffer(BitmapBuffer* buffer, int32 width, int32 height)
{
    buffer->Info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

    buffer->Width = width;
    buffer->Info.bmiHeader.biWidth = buffer->Width;

    buffer->Height = height;
    buffer->Info.bmiHeader.biHeight = -buffer->Height;

    buffer->Info.bmiHeader.biPlanes = 1;

    buffer->BytesPerPixel = 4;
    buffer->Info.bmiHeader.biBitCount = buffer->BytesPerPixel * 8;

    buffer->Info.bmiHeader.biCompression = BI_RGB;

    buffer->Pitch = buffer->Width * buffer->BytesPerPixel;

    if(buffer->Data)
    {
        VirtualFree(buffer->Data, 0, MEM_RELEASE);
    }

    buffer->Data = VirtualAlloc(0, (buffer->Width * buffer->Height) * buffer->BytesPerPixel, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

static void BlitWindowDC(HDC deviceContext, BitmapBuffer* buffer, WindowDimensions windowDims)
{
    StretchDIBits(deviceContext,
                  0, 0, windowDims.Width, windowDims.Height,
                  0, 0, buffer->Width, buffer->Height,
                  buffer->Data,
                  &buffer->Info,
                  DIB_RGB_COLORS,
                  SRCCOPY);
}

LRESULT CALLBACK WindowCallback(HWND windowHandle,
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
            DestroyWindow(windowHandle);
        } break;
        
        case WM_DESTROY:
        {
            // TODO(saeb): Window is being destroyed; perform shutdown cleanup.
            PostQuitMessage(0);
        } break;
        
        case WM_PAINT:
        {
            PAINTSTRUCT paint;
            HDC deviceContext = BeginPaint(windowHandle, &paint);

            WindowDimensions windowDims = GetWindowDimensions(windowHandle);
            BlitWindowDC(deviceContext, &Buffer, windowDims);
            
            EndPaint(windowHandle, &paint);
        } break;
        
        default:
        {
            result = DefWindowProc(windowHandle, message, wParam, lParam);
        } break;
    }
    
    return(result);
}

int WINAPI WinMain(HINSTANCE instance,
                   HINSTANCE,
                   LPSTR,
                   int)
{
    SetupBitmapBuffer(&Buffer, 1280, 720);
    
    WNDCLASSEX windowClass = {};
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
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
                                           "Afterglow",
                                           WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                           CW_USEDEFAULT, CW_USEDEFAULT,
                                           CW_USEDEFAULT, CW_USEDEFAULT,
                                           0,
                                           0,
                                           windowClass.hInstance,
                                           0);
        
        if(windowHandle)
        {
            HDC deviceContext = GetDC(windowHandle);
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

                FillScreen(&Buffer);
                XOffset++;
                
                WindowDimensions windowDims = GetWindowDimensions(windowHandle); 
                BlitWindowDC(deviceContext, &Buffer, windowDims);
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
