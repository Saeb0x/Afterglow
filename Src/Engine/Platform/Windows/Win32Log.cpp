#include "Engine/Platform/Log.h"

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

void LogPrint(StackAllocator* allocator, StringView message)
{
    Frame lowerFrameScratch = StackAllocatorGetFrame(allocator, Heap::Lower);

    // NOTE(saeb): StringAppend keeps the string null-terminated, which is what OutputDebugStringA needs.
    String line = StringReserve(allocator, message.Length + 2);
    if(StringAppend(&line, message) && StringAppend(&line, StringViewFromCString("\n")))
    {
        OutputDebugStringA(line.Data);
    }

    StackAllocatorReleaseFrame(allocator, lowerFrameScratch);
}
