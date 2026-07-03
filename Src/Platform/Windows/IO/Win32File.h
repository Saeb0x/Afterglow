#if !defined(WIN32FILE_H)

#include "Core/Types.h"
#include "Core/Arena.h"

struct ReadFileResult
{
    void* Data;
    uint64 Size;
};

ReadFileResult Win32ReadEntireFile(Arena* transient, const char* path);

#define WIN32FILE_H
#endif
