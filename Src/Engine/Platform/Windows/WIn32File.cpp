#include "Engine/Platform/File.h"

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

// NOTE(saeb): Relative paths resolve against the exe's folder, not the working directory; shortcuts, launchers and debuggers can all start the game from anywhere. Returns a null-terminated UTF-16 path on the Upper heap, or nullptr.
static const char16* Win32FileBuildPath(StackAllocator* allocator, StringView8 path)
{
    StringView16 widePath = SV8ToSV16(allocator, path);
    if(!widePath.Data)
    {
        return(nullptr);
    }

    // NOTE(saeb): Absolute paths pass through unchanged, so tools can reuse this.
    bool absolute = (widePath.Length >= 2 && widePath.Data[1] == u':') || (widePath.Length >= 1 && (widePath.Data[0] == u'\\' || widePath.Data[0] == u'/'));
    if(absolute)
    {
        return(widePath.Data);
    }

    // NOTE(saeb): MAX_PATH is also CreateFileW's own limit without a long-path manifest, so a longer exe path couldn't open files anyway.
    char16* exePath = (char16*)Allocate(allocator, Heap::Upper, MAX_PATH * sizeof(char16), alignof(char16));
    if(!exePath)
    {
        return(nullptr);
    }

    DWORD exePathLength = GetModuleFileNameW(nullptr, (LPWSTR)exePath, MAX_PATH);
    if(exePathLength == 0 || exePathLength >= MAX_PATH)
    {
        return(nullptr); // Failed, or truncated
    }

    // NOTE(saeb): Keep everything up to and including the last separator: "C:\Dev\Afterglow\Afterglow.exe" -> "C:\Dev\Afterglow\".
    usize directoryLength = exePathLength;
    while(directoryLength > 0 && exePath[directoryLength - 1] != u'\\')
    {
        --directoryLength;
    }

    char16* fullPath = (char16*)Allocate(allocator, Heap::Upper, (directoryLength + widePath.Length + 1) * sizeof(char16), alignof(char16));
    if(!fullPath)
    {
        return(nullptr);
    }

    for(usize index = 0; index < directoryLength; ++index)
    {
        fullPath[index] = exePath[index];
    }

    for(usize index = 0; index < widePath.Length; ++index)
    {
        fullPath[directoryLength + index] = widePath.Data[index];
    }

    fullPath[directoryLength + widePath.Length] = u'\0';

    return(fullPath);
}

bool FileRead(StackAllocator* allocator, Heap heap, StringView8 path, FileContents* contents)
{
    contents->Data = nullptr;
    contents->Size = 0;

    // NOTE(saeb): The path is only needed to open the file; release it before allocating the data so the two never interleave on the Upper heap.
    Frame pathScratch = GetFrame(allocator, Heap::Upper);
    const char16* fullPath = Win32FileBuildPath(allocator, path);

    HANDLE fileHandle = INVALID_HANDLE_VALUE;
    if(fullPath)
    {
        fileHandle = CreateFileW((LPCWSTR)fullPath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
    }

    ReleaseFrame(allocator, pathScratch);

    if(fileHandle == INVALID_HANDLE_VALUE)
    {
        return(false);
    }

    LARGE_INTEGER fileSize;
    if(!GetFileSizeEx(fileHandle, &fileSize))
    {
        CloseHandle(fileHandle);
        return(false);
    }

    usize size = (usize)fileSize.QuadPart;

    // NOTE(saeb): +1 for the zero terminator, which also means an empty file still gets a valid (non-null) buffer. 16-byte alignment so cooked data can be read as structs in place.
    Frame dataFrame = GetFrame(allocator, heap);
    uint8* data = (uint8*)Allocate(allocator, heap, size + 1, 16);
    if(!data)
    {
        CloseHandle(fileHandle);
        return(false);
    }

    // NOTE(saeb): ReadFile takes a 32-bit count, so read in chunks; a short read (the file shrank while reading) is a failure, not a partial success.
    usize totalRead = 0;
    while(totalRead < size)
    {
        usize remaining = size - totalRead;
        DWORD chunkSize = (remaining > 0x40000000) ? 0x40000000 : (DWORD)remaining;
        DWORD bytesRead = 0;

        if(!ReadFile(fileHandle, data + totalRead, chunkSize, &bytesRead, nullptr) || bytesRead == 0)
        {
            // NOTE(saeb): Give the allocation back; a failed read leaves the allocator exactly as it was.
            ReleaseFrame(allocator, dataFrame);
            CloseHandle(fileHandle);
            return(false);
        }

        totalRead += bytesRead;
    }

    CloseHandle(fileHandle);

    data[size] = 0;

    contents->Data = data;
    contents->Size = size;

    return(true);
}
