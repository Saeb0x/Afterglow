#include "Engine/Platform/Log.h"

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

void LogPrint(StackAllocator* allocator, StringView8 message)
{
    // NOTE(saeb): String8Reserve uses the Lower heap and SV8ToSV16 the Upper one, so take a frame on both.
    Frame lowerFrameScratch = GetFrame(allocator, Heap::Lower);
    Frame upperFrameScratch = GetFrame(allocator, Heap::Upper);

    // NOTE(saeb): + 1 for the newline, + 1 because String8Append always leaves room for its terminator.
    String8 line = String8Reserve(allocator, message.Length + 2);
    if(String8Append(&line, message) && String8Append(&line, SV8(u8"\n")))
    {
        StringView16 wideLine = SV8ToSV16(allocator, StringView8{ line.Data, line.Length });
        if(wideLine.Data)
        {
            OutputDebugStringW((LPCWSTR)wideLine.Data);
        }
    }

    ReleaseFrame(allocator, upperFrameScratch);
    ReleaseFrame(allocator, lowerFrameScratch);
}
