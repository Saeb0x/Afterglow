#if !defined(AFTERGLOW_FILE_H)
#define AFTERGLOW_FILE_H

#include <SSTL/Core/Types.h>
#include <SSTL/Core/String.h>
#include <SSTL/Memory/StackAllocator.h>

struct FileContents
{
    uint8* Data; // Followed by one zero byte (not counted in Size), so text can be read as a C string
    usize Size;
};

bool FileRead(StackAllocator* allocator, Heap heap, StringView8 path, FileContents* contents);

#endif
