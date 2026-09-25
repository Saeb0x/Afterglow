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

enum class FileReadResult : uint8
{
    Ok,
    InvalidPath, // Empty, drive-relative, root-relative, illegal characters, too long, or the exe path couldn't be resolved
    NotFound, // File or directory doesn't exist
    AccessDenied, // No permission, a sharing violation, or the path is a directory
    OutOfMemory, // Not enough room in the allocator for the path or the data
    ReadFailed // Open, size query or read failed for any other reason, including the file shrinking mid-read
};

enum class FileWriteResult : uint8
{
    Ok,
    InvalidPath, // Same rules as FileRead, or a parent directory couldn't be created
    AccessDenied, // No permission, or the target is read-only or open without delete sharing
    OutOfMemory, // Not enough room in the allocator for the path
    WriteFailed // Create, write or replace failed for any other reason (disk full, ...)
};

// NOTE(saeb): Reads the whole file into heap, 16-byte aligned. The caller owns the data and frees it by releasing a frame taken on heap before the call. Relative paths resolve against the exe's folder. Always borrows temporary space on the Upper heap for the path (freed before returning), even when heap is Lower. On failure, zeroes contents and leaves the allocator exactly as it was.
FileReadResult FileRead(StackAllocator* allocator, Heap heap, StringView8 path, FileContents* contents);

// NOTE(saeb): Writes the whole buffer to path, replacing any existing file, and creates missing parent directories. Relative paths resolve against the exe's folder. Writes "<path>.tmp" first, then renames it over the target, so a failed or interrupted write never leaves a partial file behind; on failure, the target is untouched. Borrows temporary space on the Upper heap for the path (freed before returning); the allocator is left exactly as it was.
FileWriteResult FileWrite(StackAllocator* allocator, StringView8 path, const void* data, usize size);

struct FileInfo
{
    uint64 Size; // 0 for directories
    uint64 LastWriteTime; // Platform-specific ticks; only meaningful compared with another LastWriteTime
    bool IsDirectory;
};

struct FileDirectoryEntry
{
    StringView8 Name; // UTF-8, the name only ("Player.png"); valid only during the callback
    FileInfo Info;
};

// NOTE(saeb): Return false to stop the listing early.
typedef bool FileDirectoryCallback(const FileDirectoryEntry* entry, void* userData);

// NOTE(saeb): Size, last write time and kind of a file or directory. Same path rules as FileRead. Borrows temporary space on the Upper heap for the path; the allocator is left exactly as it was.
FileReadResult FileGetInfo(StackAllocator* allocator, StringView8 path, FileInfo* info);

// NOTE(saeb): Reads exactly size bytes, starting at offset, into destination (the caller's memory; nothing is allocated for the data). A file that ends before offset + size is ReadFailed, and destination's contents are then undefined. Same path rules and memory rules as FileGetInfo.
FileReadResult FileReadRange(StackAllocator* allocator, StringView8 path, uint64 offset, void* destination, usize size);

// NOTE(saeb): Calls callback once per entry directly inside the directory path: not recursive, "." and ".." skipped, order unspecified. Each entry's name lives on the Upper heap only during its callback. The callback may allocate freely and may list other directories itself; anything it leaves on the Upper heap is released when it returns, Lower heap allocations are its own. Returns NotFound if path doesn't exist or isn't a directory. Otherwise the allocator is left as it was.
FileReadResult FileListDirectory(StackAllocator* allocator, StringView8 path, FileDirectoryCallback* callback, void* userData);

#endif
