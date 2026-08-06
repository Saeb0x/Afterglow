#if !defined(AFTERGLOW_WIN32FILE_H)
#define AFTERGLOW_WIN32FILE_H

#include "Engine/Types.h"
#include "SSTL/Memory.h"

struct ReadFileResult
{
    void* Data;
    uint64 Size;
};

ReadFileResult Win32ReadEntireFile(sstl::Arena* transient, const char* path);

#endif
