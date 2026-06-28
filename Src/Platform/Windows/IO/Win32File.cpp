#include "Win32File.h"

#include <windows.h>

ReadFileResult Win32ReadEntireFile(MemoryArena* transient, const char* path)
{
    ReadFileResult result = {};

    HANDLE file = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(file == INVALID_HANDLE_VALUE)
    {
        return(result);
    }

    LARGE_INTEGER size;
    if(GetFileSizeEx(file, &size))
    {
        uint8* buffer = (uint8*)PushSize(transient, size.QuadPart);

        DWORD bytesRead;
        if(ReadFile(file, buffer, (DWORD)size.QuadPart, &bytesRead, 0) && bytesRead == size.QuadPart)
        {
            result.Data = buffer;
            result.Size = size.QuadPart;
        }
    }

    CloseHandle(file);
    return(result);
}
