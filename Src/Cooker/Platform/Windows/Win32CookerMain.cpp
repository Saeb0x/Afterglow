#include "Cooker/Cooker.h"

#include <SSTL/Core/Utility.h>

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <stdio.h>

#define COOKER_MAX_ARGUMENTS 16 // The cooker takes four; anything past this is ignored, and extra ones already show the usage

// NOTE(saeb): Command-line paths are relative to the working directory, like any command-line tool, while FileRead resolves relative paths against the exe's folder. So every argument that isn't a --flag is made absolute here, and the portable cooker only ever sees absolute UTF-8 paths. Arguments go on the Upper heap and stay there for the whole run; every frame the cooker takes sits above them.
static StringView8 Win32CookerConvertArgument(StackAllocator* allocator, const wchar_t* argument)
{
    if(argument[0] == L'-' && argument[1] == L'-')
    {
        return(SV16ToSV8(allocator, SV16((const char16*)argument)));
    }

    wchar_t fullPath[MAX_PATH];
    DWORD length = GetFullPathNameW(argument, MAX_PATH, fullPath, nullptr);
    if(length == 0 || length >= MAX_PATH)
    {
        return(StringView8{ nullptr, 0 });
    }

    // NOTE(saeb): Drop trailing separators ("Data\" -> "Data"), so appending "/name" never doubles them; a drive root ("C:\") keeps its own.
    while(length > 3 && (fullPath[length - 1] == L'\\' || fullPath[length - 1] == L'/'))
    {
        --length;
    }

    return(SV16ToSV8(allocator, StringView16{ (const char16*)fullPath, length }));
}

int wmain(int argc, wchar_t** argv)
{
    // NOTE(saeb): Paths are printed as UTF-8.
    SetConsoleOutputCP(CP_UTF8);

    // NOTE(saeb): Sized for the largest texture: a 16384x16384 PNG decodes to 1 GiB, so big art needs a bigger block; 256 MiB covers 4096x4096 with room to spare.
    StackAllocator memory;
    if(!InitStackAllocator(&memory, SSTL_MIB(256)))
    {
        printf("error: couldn't allocate cooker memory\n");
        return(1);
    }

    StringView8 arguments[COOKER_MAX_ARGUMENTS];
    int argumentCount = 0;

    // NOTE(saeb): argv[0] is the exe itself.
    for(int index = 1; index < argc && argumentCount < COOKER_MAX_ARGUMENTS; ++index)
    {
        StringView8 converted = Win32CookerConvertArgument(&memory, argv[index]);
        if(!converted.Data)
        {
            printf("error: argument %d is an invalid or too long path\n", index);
            ShutdownStackAllocator(&memory);
            return(1);
        }

        arguments[argumentCount++] = converted;
    }

    int result = CookerRun(&memory, argumentCount, arguments);

    ShutdownStackAllocator(&memory);

    return(result);
}
