#if !defined(AFTERGLOW_LOG_H)
#define AFTERGLOW_LOG_H

#include <Creuse/Core/String.h>
#include <Creuse/Memory/StackAllocator.h>

// NOTE(saeb): The output goes to the debugger's output window (for now).
void LogPrint(StackAllocator* allocator, StringView message);

#endif
