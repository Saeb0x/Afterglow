#if !defined(AFTERGLOW_WIN32INPUT_H)
#define AFTERGLOW_WIN32INPUT_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

void Win32InputBegin();
void Win32InputProcess(UINT message, WPARAM wParam, LPARAM lParam);

#endif
