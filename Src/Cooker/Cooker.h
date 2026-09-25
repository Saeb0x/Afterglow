#if !defined(AFTERGLOW_COOKER_H)
#define AFTERGLOW_COOKER_H

#include <SSTL/Core/Types.h>
#include <SSTL/Core/String.h>
#include <SSTL/Memory/StackAllocator.h>

#include "Engine/Platform/File.h"

struct CookerContext
{
    StackAllocator* Memory; // Owned by the platform entry point
    bool Debug; // Shaders keep debug info and skip optimization, so RenderDoc / PIX shows the HLSL
};

// NOTE(saeb): The portable cooker. The platform entry point owns the allocator, converts the command line to UTF-8, makes every path argument absolute, and calls this with the arguments after the program name. Returns the process exit code: 0 when everything cooked or was up to date.
int CookerRun(StackAllocator* memory, int argumentCount, const StringView8* arguments);

// NOTE(saeb): Prints "path: error: message", MSVC's format, so the path is clickable in Visual Studio and most terminals. A null path prints just the message. The message is a printf format.
void CookerError(StringView8 path, const char* format, ...);

// NOTE(saeb): Short reasons for error messages: "not found", "access denied", ...
const char* CookerDescribeRead(FileReadResult result);
const char* CookerDescribeWrite(FileWriteResult result);

// NOTE(saeb): Both paths are absolute UTF-8; the output's parent folders may not exist yet (FileWrite creates them). Each prints its own error and returns false on failure. The caller wraps every call in Lower and Upper frames, so a cook can allocate freely and never needs to clean up.
bool CookTexture(CookerContext* context, StringView8 sourcePath, StringView8 outputPath);
bool CookShader(CookerContext* context, StringView8 sourcePath, StringView8 outputPath);

#endif
