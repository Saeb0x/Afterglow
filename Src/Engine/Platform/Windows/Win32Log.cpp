#include "Engine/Platform/Log.h"

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

void LogPrint(StackAllocator* allocator, StringView8 message)
{
    Frame lowerFrameScratch = GetFrame(allocator, Heap::Lower);
    Frame upperFrameScratch = GetFrame(allocator, Heap::Upper);

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
