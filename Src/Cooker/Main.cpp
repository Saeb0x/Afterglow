#include "Cooker.h"

#include "Engine/Asset/AssetFormat.h"

#include <SSTL/Core/Utility.h>

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <stdarg.h>
#include <stdio.h>
#include <wchar.h>

// NOTE(saeb): Paths stay UTF-16 while walking, since that's what Windows gives us, and are converted to UTF-8 only to call the File API. MAX_PATH is also CreateFileW's own limit without a long-path manifest.
struct CookerPath
{
    char16 Data[MAX_PATH];
    usize Length;
};

enum class CookerSourceType : uint8
{
    Unknown,
    Texture,
    Shader
};

struct CookerWalk
{
    CookerContext* Context;
    CookerPath Source;
    CookerPath Output;
    usize SourceRootLength8; // UTF-8 length of the source root, to print paths relative to it
    bool Force;
    uint32 Cooked;
    uint32 UpToDate;
    uint32 Failed;
};

static CookerContext Context;

void CookerError(StringView8 path, const char* format, ...)
{
    if(path.Data)
    {
        printf("%.*s: error: ", (int)path.Length, (const char*)path.Data);
    }
    else
    {
        printf("error: ");
    }

    va_list arguments;
    va_start(arguments, format);
    vprintf(format, arguments);
    va_end(arguments);

    printf("\n");
}

const char* CookerDescribeRead(FileReadResult result)
{
    switch(result)
    {
        case FileReadResult::Ok:
        {
            return("ok");
        }

        case FileReadResult::InvalidPath:
        {
            return("invalid path");
        }

        case FileReadResult::NotFound:
        {
            return("not found");
        }

        case FileReadResult::AccessDenied:
        {
            return("access denied");
        }

        case FileReadResult::OutOfMemory:
        {
            return("out of memory");
        }

        case FileReadResult::ReadFailed:
        {
            return("read failed");
        }
    }

    return("unknown error");
}

const char* CookerDescribeWrite(FileWriteResult result)
{
    switch(result)
    {
        case FileWriteResult::Ok:
        {
            return("ok");
        }

        case FileWriteResult::InvalidPath:
        {
            return("invalid path");
        }

        case FileWriteResult::AccessDenied:
        {
            return("access denied");
        }

        case FileWriteResult::OutOfMemory:
        {
            return("out of memory");
        }

        case FileWriteResult::WriteFailed:
        {
            return("write failed");
        }
    }

    return("unknown error");
}

static void CookerErrorWide(const char16* path, const char* message)
{
    Frame scratch = GetFrame(&Context.Memory, Heap::Upper);
    CookerError(SV16ToSV8(&Context.Memory, SV16(path)), "%s", message);
    ReleaseFrame(&Context.Memory, scratch);
}

static void CookerPathTruncate(CookerPath* path, usize length)
{
    path->Length = length;
    path->Data[length] = u'\0';
}

// NOTE(saeb): On failure (too long), the path is left partly appended; callers always truncate back to a saved length.
static bool CookerPathAppend(CookerPath* path, const char16* text)
{
    for(usize index = 0; text[index] != u'\0'; ++index)
    {
        if(path->Length + 1 >= MAX_PATH) // Keep room for the terminator
        {
            return(false);
        }

        path->Data[path->Length++] = text[index];
    }

    path->Data[path->Length] = u'\0';

    return(true);
}

// NOTE(saeb): Resolved against the working directory like any command-line tool, so everything after this is absolute; FileRead would otherwise resolve relative paths against the exe's folder.
static bool CookerPathResolve(const wchar_t* argument, CookerPath* path)
{
    DWORD length = GetFullPathNameW(argument, MAX_PATH, (LPWSTR)path->Data, nullptr);
    if(length == 0 || length >= MAX_PATH)
    {
        return(false);
    }

    // NOTE(saeb): Drop trailing separators ("Data\" -> "Data"), so appending "\name" never doubles them.
    while(length > 3 && (path->Data[length - 1] == u'\\' || path->Data[length - 1] == u'/'))
    {
        --length;
    }

    CookerPathTruncate(path, length);

    return(true);
}

// NOTE(saeb): Case-insensitive, so "Something.PNG" still cooks.
static bool CookerEndsWith(const char16* name, const char16* suffix)
{
    usize nameLength = 0;
    while(name[nameLength] != u'\0')
    {
        ++nameLength;
    }

    usize suffixLength = 0;
    while(suffix[suffixLength] != u'\0')
    {
        ++suffixLength;
    }

    if(suffixLength > nameLength)
    {
        return(false);
    }

    const char16* tail = name + (nameLength - suffixLength);
    for(usize index = 0; index < suffixLength; ++index)
    {
        char16 c = tail[index];
        if(c >= u'A' && c <= u'Z')
        {
            c = (char16)(c + (u'a' - u'A'));
        }

        if(c != suffix[index])
        {
            return(false);
        }
    }

    return(true);
}

static CookerSourceType CookerGetSourceType(const char16* name)
{
    if(CookerEndsWith(name, u".png"))
    {
        return(CookerSourceType::Texture);
    }

    if(CookerEndsWith(name, u".hlsl"))
    {
        return(CookerSourceType::Shader);
    }

    return(CookerSourceType::Unknown);
}

static bool CookerGetWriteTime(const char16* path, FILETIME* time)
{
    WIN32_FILE_ATTRIBUTE_DATA data;
    if(!GetFileAttributesExW((LPCWSTR)path, GetFileExInfoStandard, &data))
    {
        return(false);
    }

    *time = data.ftLastWriteTime;

    return(true);
}

// NOTE(saeb): An output only counts if it was cooked with the current format; the engine rejects any other version, so an old one must be recooked even when it's newer than its source.
static bool CookerOutputIsCurrent(const char16* path)
{
    HANDLE file = CreateFileW((LPCWSTR)path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if(file == INVALID_HANDLE_VALUE)
    {
        return(false);
    }

    AssetHeader header = {};
    DWORD bytesRead = 0;
    bool read = ReadFile(file, &header, sizeof(header), &bytesRead, nullptr) && bytesRead == sizeof(header);
    CloseHandle(file);

    return(read && header.Magic == AG_ASSET_MAGIC && header.Version == AG_ASSET_VERSION);
}

static void CookerCookFile(CookerWalk* walk, CookerSourceType type, FILETIME sourceTime)
{
    // NOTE(saeb): Up to date if the output is newer than its source and was cooked with the current format version. The timestamp check comes first, since it needs no file read. When a cooker change alters the output, bump AG_ASSET_VERSION (or cook with --force).
    FILETIME outputTime;
    bool upToDate = !walk->Force && CookerGetWriteTime(walk->Output.Data, &outputTime) && CompareFileTime(&outputTime, &sourceTime) > 0 && CookerOutputIsCurrent(walk->Output.Data);

    if(upToDate)
    {
        ++walk->UpToDate;
        return;
    }

    // NOTE(saeb): A fresh frame on both heaps per file; whatever a cook allocates is gone before the next one, so memory use stays flat.
    Frame lowerFrame = GetFrame(&Context.Memory, Heap::Lower);
    Frame upperFrame = GetFrame(&Context.Memory, Heap::Upper);

    StringView8 sourcePath = SV16ToSV8(&Context.Memory, StringView16{ walk->Source.Data, walk->Source.Length });
    StringView8 outputPath = SV16ToSV8(&Context.Memory, StringView16{ walk->Output.Data, walk->Output.Length });

    bool cooked = false;
    if(sourcePath.Data && outputPath.Data)
    {
        if(type == CookerSourceType::Texture)
        {
            cooked = CookTexture(&Context, sourcePath, outputPath);
        }
        else
        {
            cooked = CookShader(&Context, sourcePath, outputPath);
        }

        if(cooked)
        {
            // NOTE(saeb): Relative to the source root, skipping the separator: "Sprites\Player.png".
            usize skip = walk->SourceRootLength8 + 1;
            printf("Cooked %.*s\n", (int)(sourcePath.Length - skip), (const char*)sourcePath.Data + skip);
        }
    }
    else
    {
        CookerErrorWide(walk->Source.Data, "out of memory converting the path");
    }

    if(cooked)
    {
        ++walk->Cooked;
    }
    else
    {
        ++walk->Failed;
    }

    ReleaseFrame(&Context.Memory, upperFrame);
    ReleaseFrame(&Context.Memory, lowerFrame);
}

static void CookerWalkDirectory(CookerWalk* walk)
{
    usize sourceLength = walk->Source.Length;
    usize outputLength = walk->Output.Length;

    if(!CookerPathAppend(&walk->Source, u"\\*"))
    {
        CookerPathTruncate(&walk->Source, sourceLength);
        CookerErrorWide(walk->Source.Data, "path too long");
        ++walk->Failed;
        return;
    }

    WIN32_FIND_DATAW find;
    HANDLE findHandle = FindFirstFileW((LPCWSTR)walk->Source.Data, &find);
    CookerPathTruncate(&walk->Source, sourceLength);

    // NOTE(saeb): Even an empty folder lists "." and "..", so failing here is a real error.
    if(findHandle == INVALID_HANDLE_VALUE)
    {
        CookerErrorWide(walk->Source.Data, "couldn't open directory");
        ++walk->Failed;
        return;
    }

    do
    {
        const char16* name = (const char16*)find.cFileName;

        // NOTE(saeb): Skips ".", "..", and dot-files and folders such as .gitkeep.
        if(name[0] == u'.')
        {
            continue;
        }

        bool isDirectory = (find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        CookerSourceType type = isDirectory ? CookerSourceType::Unknown : CookerGetSourceType(name);
        if(!isDirectory && type == CookerSourceType::Unknown)
        {
            continue;
        }

        bool pathsFit = CookerPathAppend(&walk->Source, u"\\") && CookerPathAppend(&walk->Source, name) && CookerPathAppend(&walk->Output, u"\\") && CookerPathAppend(&walk->Output, name);

        if(pathsFit && !isDirectory)
        {
            // NOTE(saeb): Replace the extension: "Player.png" -> "Player.aga". A known type guarantees there's a dot.
            usize dot = walk->Output.Length;
            while(walk->Output.Data[dot - 1] != u'.')
            {
                --dot;
            }

            CookerPathTruncate(&walk->Output, dot - 1);
            pathsFit = CookerPathAppend(&walk->Output, u".aga");
        }

        if(!pathsFit)
        {
            CookerPathTruncate(&walk->Source, sourceLength);
            CookerErrorWide(walk->Source.Data, "path too long inside this directory");
            ++walk->Failed;
        }
        else if(isDirectory)
        {
            CookerWalkDirectory(walk);
        }
        else
        {
            CookerCookFile(walk, type, find.ftLastWriteTime);
        }

        CookerPathTruncate(&walk->Source, sourceLength);
        CookerPathTruncate(&walk->Output, outputLength);
    }
    while(FindNextFileW(findHandle, &find));

    FindClose(findHandle);
}

int wmain(int argc, wchar_t** argv)
{
    // NOTE(saeb): Paths are printed as UTF-8.
    SetConsoleOutputCP(CP_UTF8);

    const wchar_t* sourceArgument = nullptr;
    const wchar_t* outputArgument = nullptr;
    bool force = false;

    for(int index = 1; index < argc; ++index)
    {
        if(wcscmp(argv[index], L"--debug") == 0)
        {
            Context.Debug = true;
        }
        else if(wcscmp(argv[index], L"--force") == 0)
        {
            force = true;
        }
        else if(!sourceArgument)
        {
            sourceArgument = argv[index];
        }
        else if(!outputArgument)
        {
            outputArgument = argv[index];
        }
        else
        {
            sourceArgument = nullptr; // Too many arguments; show usage
            break;
        }
    }

    if(!sourceArgument || !outputArgument)
    {
        printf("Usage: AfterglowCooker.exe <SourceDir> <OutputDir> [--debug] [--force]\n");
        return(1);
    }

    // NOTE(saeb): Sized for the largest texture: a 16384x16384 PNG decodes to 1 GiB, so big art needs a bigger block; 256 MiB covers 4096x4096 with room to spare.
    if(!InitStackAllocator(&Context.Memory, SSTL_MIB(256)))
    {
        CookerError(StringView8{ nullptr, 0 }, "couldn't allocate cooker memory");
        return(1);
    }

    CookerWalk walk = {};
    walk.Context = &Context;
    walk.Force = force;

    if(!CookerPathResolve(sourceArgument, &walk.Source) || !CookerPathResolve(outputArgument, &walk.Output))
    {
        CookerError(StringView8{ nullptr, 0 }, "invalid or too long source/output directory");
        ShutdownStackAllocator(&Context.Memory);
        return(1);
    }

    DWORD sourceAttributes = GetFileAttributesW((LPCWSTR)walk.Source.Data);
    if(sourceAttributes == INVALID_FILE_ATTRIBUTES || !(sourceAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
        CookerErrorWide(walk.Source.Data, "source directory doesn't exist");
        ShutdownStackAllocator(&Context.Memory);
        return(1);
    }

    walk.SourceRootLength8 = SV16ToSV8Length(StringView16{ walk.Source.Data, walk.Source.Length });

    CookerWalkDirectory(&walk);

    printf("[Afterglow] %u cooked, %u up to date, %u failed.\n", walk.Cooked, walk.UpToDate, walk.Failed);

    ShutdownStackAllocator(&Context.Memory);

    return((walk.Failed > 0) ? 1 : 0);
}
