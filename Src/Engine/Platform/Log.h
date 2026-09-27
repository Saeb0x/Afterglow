#if !defined(AFTERGLOW_LOG_H)
#define AFTERGLOW_LOG_H

#include <SSTL/Core/String.h>
#include <SSTL/Memory/StackAllocator.h>

// NOTE(saeb): The output goes to the debugger's output window (for now).
void LogPrint(StackAllocator* allocator, StringView8 message);

#endif
