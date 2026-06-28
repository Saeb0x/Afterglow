#if !defined(WIN32FILE_H)

#include "Core/Types.h"
#include "Core/MemoryArena.h"

struct ReadFileResult
{
    void* Data;
    uint64 Size;
};

ReadFileResult Win32ReadEntireFile(MemoryArena* transient, const char* path);

#define WIN32FILE_H
#endif
