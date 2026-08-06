#if !defined(AFTERGLOW_WIN32INPUT_H)
#define AFTERGLOW_WIN32INPUT_H

#include "Engine/Input.h"
#include <windows.h>

void Win32BeginInputFrame(GameInput* input);
void Win32ProcessInputMessage(GameInput* input, UINT message, WPARAM wParam, LPARAM lParam);

#endif
