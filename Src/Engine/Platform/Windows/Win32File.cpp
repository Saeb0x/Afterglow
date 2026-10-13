#include "Engine/Platform/File.h"

#include <Creuse/Core/Utility.h>

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

static bool Win32FileIsSeparator(char c)
{
    return(c == '\\' || c == '/');
}

// NOTE(saeb): Relative paths resolve against the exe's folder, not the working directory; shortcuts, launchers and debuggers can all start the game from anywhere. Writes a null-terminated ASCII path on the Upper heap to *fullPath.
static FileReadResult Win32FileBuildPath(StackAllocator* allocator, StringView path, const char** fullPath)
{
    *fullPath = nullptr;

    // NOTE(saeb): An empty path would resolve to the exe's folder itself, which CreateFileA rejects as access denied.
    if(path.Length == 0)
    {
        return(FileReadResult::InvalidPath);
    }

    // NOTE(saeb): A view isn't null-terminated, and Windows wants paths that are, so copy it.
    usize length = path.Length;
    char* text = (char*)StackAllocatorAllocate(allocator, Heap::Upper, length + 1, alignof(char));
    if(!text)
    {
        return(FileReadResult::OutOfMemory);
    }

    for(usize index = 0; index < length; ++index)
    {
        text[index] = path.Data[index];
    }

    text[length] = '\0';

    // NOTE(saeb): Only "C:\..." and "\\..." (UNC, \\?\) are truly absolute and pass through unchanged, so tools can reuse this. "C:something" and "\something" still depend on the working directory's drive or folder, so reject them rather than guess.
    bool hasDrive = (length >= 2 && text[1] == ':');
    if(hasDrive)
    {
        if(length >= 3 && Win32FileIsSeparator(text[2]))
        {
            *fullPath = text;
            return(FileReadResult::Ok);
        }

        return(FileReadResult::InvalidPath); // Drive-relative
    }

    if(length >= 1 && Win32FileIsSeparator(text[0]))
    {
        if(length >= 2 && Win32FileIsSeparator(text[1]))
        {
            *fullPath = text;
            return(FileReadResult::Ok);
        }

        return(FileReadResult::InvalidPath); // Root-relative
    }

    // NOTE(saeb): MAX_PATH is also CreateFileA's own limit, so a longer exe path couldn't open files anyway.
    char* exePath = (char*)StackAllocatorAllocate(allocator, Heap::Upper, MAX_PATH * sizeof(char), alignof(char));
    if(!exePath)
    {
        return(FileReadResult::OutOfMemory);
    }

    DWORD exePathLength = GetModuleFileNameA(nullptr, exePath, MAX_PATH);
    if(exePathLength == 0 || exePathLength >= MAX_PATH)
    {
        return(FileReadResult::InvalidPath); // Failed, or truncated
    }

    // NOTE(saeb): Keep everything up to and including the last separator: "C:\Dev\Afterglow\Afterglow.exe" -> "C:\Dev\Afterglow\".
    usize directoryLength = exePathLength;
    while(directoryLength > 0 && exePath[directoryLength - 1] != '\\')
    {
        --directoryLength;
    }

    char* result = (char*)StackAllocatorAllocate(allocator, Heap::Upper, (directoryLength + length + 1) * sizeof(char), alignof(char));
    if(!result)
    {
        return(FileReadResult::OutOfMemory);
    }

    for(usize index = 0; index < directoryLength; ++index)
    {
        result[index] = exePath[index];
    }

    for(usize index = 0; index < length; ++index)
    {
        result[directoryLength + index] = text[index];
    }

    result[directoryLength + length] = '\0';
    *fullPath = result;

    return(FileReadResult::Ok);
}

static FileReadResult Win32FileReadResultFromError(DWORD error)
{
    switch(error)
    {
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
        case ERROR_DIRECTORY: // A file where a directory was expected
        {
            return(FileReadResult::NotFound);
        }

        case ERROR_INVALID_NAME:
        case ERROR_FILENAME_EXCED_RANGE:
        {
            return(FileReadResult::InvalidPath);
        }

        case ERROR_ACCESS_DENIED:
        case ERROR_SHARING_VIOLATION:
        {
            return(FileReadResult::AccessDenied);
        }

        default:
        {
            return(FileReadResult::ReadFailed);
        }
    }
}

// NOTE(saeb): FILETIME is 100-nanosecond ticks since 1601; as one number it compares like any integer.
static uint64 Win32FileTimeToTicks(FILETIME time)
{
    return(((uint64)time.dwHighDateTime << 32) | (uint64)time.dwLowDateTime);
}

// NOTE(saeb): Same sharing as FileRead, so a file an editor still has open can be read.
static FileReadResult Win32FileOpenForRead(StackAllocator* allocator, StringView path, HANDLE* fileHandle)
{
    *fileHandle = INVALID_HANDLE_VALUE;

    Frame pathScratch = StackAllocatorGetFrame(allocator, Heap::Upper);

    const char* fullPath = nullptr;
    FileReadResult result = Win32FileBuildPath(allocator, path, &fullPath);
    if(result == FileReadResult::Ok)
    {
        *fileHandle = CreateFileA(fullPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if(*fileHandle == INVALID_HANDLE_VALUE)
        {
            result = Win32FileReadResultFromError(GetLastError());
        }
    }

    StackAllocatorReleaseFrame(allocator, pathScratch);

    return(result);
}

FileReadResult FileRead(StackAllocator* allocator, Heap heap, StringView path, FileContents* contents)
{
    contents->Data = nullptr;
    contents->Size = 0;

    // NOTE(saeb): The path is only needed to open the file; release it before allocating the data so the two never interleave on the Upper heap.
    Frame pathScratch = StackAllocatorGetFrame(allocator, Heap::Upper);

    const char* fullPath = nullptr;
    FileReadResult pathResult = Win32FileBuildPath(allocator, path, &fullPath);

    HANDLE fileHandle = INVALID_HANDLE_VALUE;
    DWORD openError = 0;
    if(pathResult == FileReadResult::Ok)
    {
        // NOTE(saeb): Share everything so the open doesn't fail while a tool or editor still has the file open. The file can then change mid-read; a shrink is caught by the short-read check below, but an in-place rewrite of the same size isn't.
        fileHandle = CreateFileA(fullPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
        openError = GetLastError(); // Before ReleaseFrame, so nothing in between can overwrite it
    }

    StackAllocatorReleaseFrame(allocator, pathScratch);

    if(pathResult != FileReadResult::Ok)
    {
        return(pathResult);
    }

    if(fileHandle == INVALID_HANDLE_VALUE)
    {
        return(Win32FileReadResultFromError(openError));
    }

    LARGE_INTEGER fileSize;
    if(!GetFileSizeEx(fileHandle, &fileSize))
    {
        CloseHandle(fileHandle);
        return(FileReadResult::ReadFailed);
    }

    usize size = (usize)fileSize.QuadPart;

    // NOTE(saeb): +1 for the zero terminator, which also means an empty file still gets a valid (non-null) buffer. 16-byte alignment so cooked data can be read as structs in place.
    Frame dataFrame = StackAllocatorGetFrame(allocator, heap);
    uint8* data = (uint8*)StackAllocatorAllocate(allocator, heap, size + 1, 16);
    if(!data)
    {
        CloseHandle(fileHandle);
        return(FileReadResult::OutOfMemory);
    }

    // NOTE(saeb): ReadFile takes a 32-bit count, so read in chunks; a short read (the file shrank while reading) is a failure, not a partial success.
    usize totalRead = 0;
    while(totalRead < size)
    {
        usize remaining = size - totalRead;
        DWORD chunkSize = (remaining > 0x40000000) ? 0x40000000 : (DWORD)remaining;
        DWORD bytesRead = 0;

        if(!ReadFile(fileHandle, data + totalRead, chunkSize, &bytesRead, nullptr) || bytesRead == 0)
        {
            // NOTE(saeb): Give the allocation back; a failed read leaves the allocator exactly as it was.
            StackAllocatorReleaseFrame(allocator, dataFrame);
            CloseHandle(fileHandle);
            return(FileReadResult::ReadFailed);
        }

        totalRead += bytesRead;
    }

    CloseHandle(fileHandle);

    data[size] = 0;

    contents->Data = data;
    contents->Size = size;

    return(FileReadResult::Ok);
}

// NOTE(saeb): Creates every directory along path: "C:\A\B\File.aga" -> "C:\A", then "C:\A\B". Failures are ignored; the root, "\\something" and existing folders all fail harmlessly, and a folder that really couldn't be created makes the CreateFileA after this fail instead.
static void Win32FileCreateParentDirectories(char* path)
{
    for(usize index = 1; path[index] != '\0'; ++index)
    {
        if(Win32FileIsSeparator(path[index]))
        {
            char separator = path[index];
            path[index] = '\0';
            CreateDirectoryA(path, nullptr);
            path[index] = separator;
        }
    }
}

static FileWriteResult Win32FileWriteResultFromError(DWORD error)
{
    switch(error)
    {
        case ERROR_PATH_NOT_FOUND:
        case ERROR_INVALID_NAME:
        case ERROR_FILENAME_EXCED_RANGE:
        {
            return(FileWriteResult::InvalidPath);
        }

        case ERROR_ACCESS_DENIED:
        case ERROR_SHARING_VIOLATION:
        {
            return(FileWriteResult::AccessDenied);
        }

        default:
        {
            return(FileWriteResult::WriteFailed);
        }
    }
}

// NOTE(saeb): All allocations here are Upper heap scratch; FileWrite releases them in one place, so every early return below is safe.
static FileWriteResult Win32FileWrite(StackAllocator* allocator, StringView path, const void* data, usize size)
{
    if(!data && size > 0)
    {
        return(FileWriteResult::WriteFailed);
    }

    const char* fullPath = nullptr;
    FileReadResult pathResult = Win32FileBuildPath(allocator, path, &fullPath);
    if(pathResult == FileReadResult::OutOfMemory)
    {
        return(FileWriteResult::OutOfMemory);
    }

    if(pathResult != FileReadResult::Ok)
    {
        return(FileWriteResult::InvalidPath);
    }

    usize fullPathLength = 0;
    while(fullPath[fullPathLength] != '\0')
    {
        ++fullPathLength;
    }

    static const char TempSuffix[] = ".tmp";
    usize suffixLength = CREUSE_ARRAYCOUNT(TempSuffix) - 1;

    char* tempPath = (char*)StackAllocatorAllocate(allocator, Heap::Upper, (fullPathLength + suffixLength + 1) * sizeof(char), alignof(char));
    if(!tempPath)
    {
        return(FileWriteResult::OutOfMemory);
    }

    for(usize index = 0; index < fullPathLength; ++index)
    {
        tempPath[index] = fullPath[index];
    }

    for(usize index = 0; index < suffixLength; ++index)
    {
        tempPath[fullPathLength + index] = TempSuffix[index];
    }

    tempPath[fullPathLength + suffixLength] = '\0';

    // NOTE(saeb): The temp file sits next to the target, so both share the same parent directories.
    Win32FileCreateParentDirectories(tempPath);

    // NOTE(saeb): No sharing; nobody should read a half-written file.
    HANDLE fileHandle = CreateFileA(tempPath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
    if(fileHandle == INVALID_HANDLE_VALUE)
    {
        return(Win32FileWriteResultFromError(GetLastError()));
    }

    // NOTE(saeb): WriteFile takes a 32-bit count, so write in chunks; anything short of the full chunk is a failure (disk full, ...).
    const uint8* bytes = (const uint8*)data;
    usize totalWritten = 0;
    bool written = true;
    while(totalWritten < size)
    {
        usize remaining = size - totalWritten;
        DWORD chunkSize = (remaining > 0x40000000) ? 0x40000000 : (DWORD)remaining;
        DWORD bytesWritten = 0;

        if(!WriteFile(fileHandle, bytes + totalWritten, chunkSize, &bytesWritten, nullptr) || bytesWritten != chunkSize)
        {
            written = false;
            break;
        }

        totalWritten += bytesWritten;
    }

    CloseHandle(fileHandle);

    if(!written)
    {
        DeleteFileA(tempPath);
        return(FileWriteResult::WriteFailed);
    }

    // NOTE(saeb): The rename replaces the target in one step; readers see the old file or the new one, never a mix. Fails with AccessDenied while anyone has the target open; the target is left untouched, so retrying is safe.
    if(!MoveFileExA(tempPath, fullPath, MOVEFILE_REPLACE_EXISTING))
    {
        DWORD moveError = GetLastError(); // Before DeleteFileA, which would overwrite it
        DeleteFileA(tempPath);
        return(Win32FileWriteResultFromError(moveError));
    }

    return(FileWriteResult::Ok);
}

FileWriteResult FileWrite(StackAllocator* allocator, StringView path, const void* data, usize size)
{
    Frame pathScratch = StackAllocatorGetFrame(allocator, Heap::Upper);
    FileWriteResult result = Win32FileWrite(allocator, path, data, size);
    StackAllocatorReleaseFrame(allocator, pathScratch);

    return(result);
}

FileReadResult FileGetInfo(StackAllocator* allocator, StringView path, FileInfo* info)
{
    info->Size = 0;
    info->LastWriteTime = 0;
    info->IsDirectory = false;

    Frame pathScratch = StackAllocatorGetFrame(allocator, Heap::Upper);

    const char* fullPath = nullptr;
    FileReadResult result = Win32FileBuildPath(allocator, path, &fullPath);
    if(result == FileReadResult::Ok)
    {
        WIN32_FILE_ATTRIBUTE_DATA data;
        if(GetFileAttributesExA(fullPath, GetFileExInfoStandard, &data))
        {
            info->IsDirectory = (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
            info->Size = info->IsDirectory ? 0 : (((uint64)data.nFileSizeHigh << 32) | (uint64)data.nFileSizeLow);
            info->LastWriteTime = Win32FileTimeToTicks(data.ftLastWriteTime);
        }
        else
        {
            result = Win32FileReadResultFromError(GetLastError());
        }
    }

    StackAllocatorReleaseFrame(allocator, pathScratch);

    return(result);
}

FileReadResult FileReadRange(StackAllocator* allocator, StringView path, uint64 offset, void* destination, usize size)
{
    // NOTE(saeb): SetFilePointerEx takes a signed 64-bit position.
    if((!destination && size > 0) || offset > 0x7FFFFFFFFFFFFFFFull)
    {
        return(FileReadResult::ReadFailed);
    }

    HANDLE fileHandle = INVALID_HANDLE_VALUE;
    FileReadResult result = Win32FileOpenForRead(allocator, path, &fileHandle);
    if(result != FileReadResult::Ok)
    {
        return(result);
    }

    LARGE_INTEGER position;
    position.QuadPart = (LONGLONG)offset;
    if(!SetFilePointerEx(fileHandle, position, nullptr, FILE_BEGIN))
    {
        CloseHandle(fileHandle);
        return(FileReadResult::ReadFailed);
    }

    // NOTE(saeb): Same chunked loop as FileRead; reading past the end returns 0 bytes, so a file shorter than offset + size fails here.
    uint8* bytes = (uint8*)destination;
    usize totalRead = 0;
    while(totalRead < size)
    {
        usize remaining = size - totalRead;
        DWORD chunkSize = (remaining > 0x40000000) ? 0x40000000 : (DWORD)remaining;
        DWORD bytesRead = 0;

        if(!ReadFile(fileHandle, bytes + totalRead, chunkSize, &bytesRead, nullptr) || bytesRead == 0)
        {
            CloseHandle(fileHandle);
            return(FileReadResult::ReadFailed);
        }

        totalRead += bytesRead;
    }

    CloseHandle(fileHandle);

    return(FileReadResult::Ok);
}

// NOTE(saeb): The search pattern ("<path>\*") and the find handle live for the whole listing; everything a callback does happens above them on the Upper heap.
static FileReadResult Win32FileListDirectory(StackAllocator* allocator, StringView path, FileDirectoryCallback* callback, void* userData)
{
    const char* fullPath = nullptr;
    FileReadResult result = Win32FileBuildPath(allocator, path, &fullPath);
    if(result != FileReadResult::Ok)
    {
        return(result);
    }

    usize fullPathLength = 0;
    while(fullPath[fullPathLength] != '\0')
    {
        ++fullPathLength;
    }

    // NOTE(saeb): "Data" -> "Data\*", and "Data\" -> "Data\*" (no doubled separator).
    bool endsWithSeparator = fullPathLength > 0 && Win32FileIsSeparator(fullPath[fullPathLength - 1]);
    usize patternLength = fullPathLength + (endsWithSeparator ? 1 : 2);

    char* pattern = (char*)StackAllocatorAllocate(allocator, Heap::Upper, (patternLength + 1) * sizeof(char), alignof(char));
    if(!pattern)
    {
        return(FileReadResult::OutOfMemory);
    }

    for(usize index = 0; index < fullPathLength; ++index)
    {
        pattern[index] = fullPath[index];
    }

    if(!endsWithSeparator)
    {
        pattern[fullPathLength] = '\\';
    }

    pattern[patternLength - 1] = '*';
    pattern[patternLength] = '\0';

    // NOTE(saeb): FindExInfoBasic skips the 8.3 short name, which nothing here uses.
    WIN32_FIND_DATAA find;
    HANDLE findHandle = FindFirstFileExA(pattern, FindExInfoBasic, &find, FindExSearchNameMatch, nullptr, 0);
    if(findHandle == INVALID_HANDLE_VALUE)
    {
        return(Win32FileReadResultFromError(GetLastError()));
    }

    // NOTE(saeb): True when the loop ends early (callback said stop, or out of memory) rather than because FindNextFileA ran out.
    bool stoppedEarly = false;

    do
    {
        const char* name = (const char*)find.cFileName;

        bool dot = (name[0] == '.' && name[1] == '\0');
        bool dotDot = (name[0] == '.' && name[1] == '.' && name[2] == '\0');
        if(dot || dotDot)
        {
            continue;
        }

        Frame entryScratch = StackAllocatorGetFrame(allocator, Heap::Upper);

        FileDirectoryEntry entry;
        entry.Name = StringViewFromCString(name);

        entry.Info.IsDirectory = (find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        entry.Info.Size = entry.Info.IsDirectory ? 0 : (((uint64)find.nFileSizeHigh << 32) | (uint64)find.nFileSizeLow);
        entry.Info.LastWriteTime = Win32FileTimeToTicks(find.ftLastWriteTime);

        bool keepGoing = callback(&entry, userData);

        StackAllocatorReleaseFrame(allocator, entryScratch);

        if(!keepGoing)
        {
            stoppedEarly = true;
            break;
        }
    }
    while(FindNextFileA(findHandle, &find));

    // NOTE(saeb): Otherwise the loop ended because FindNextFileA failed, and only "no more files" is a normal end; anything else means the listing is incomplete. Read right after the loop, before FindClose can overwrite it.
    if(!stoppedEarly && GetLastError() != ERROR_NO_MORE_FILES)
    {
        result = FileReadResult::ReadFailed;
    }

    FindClose(findHandle);

    return(result);
}

FileReadResult FileListDirectory(StackAllocator* allocator, StringView path, FileDirectoryCallback* callback, void* userData)
{
    if(!callback)
    {
        return(FileReadResult::ReadFailed);
    }

    Frame pathScratch = StackAllocatorGetFrame(allocator, Heap::Upper);
    FileReadResult result = Win32FileListDirectory(allocator, path, callback, userData);
    StackAllocatorReleaseFrame(allocator, pathScratch);

    return(result);
}
