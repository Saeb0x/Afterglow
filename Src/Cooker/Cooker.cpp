#include "Cooker.h"

#include "Engine/Asset/AssetFormat.h"

#include <stdarg.h>
#include <stdio.h>

// NOTE(saeb): UTF-8 bytes. Only a buffer size; the platform's own limit (MAX_PATH on Windows) is lower and comes back from the File layer as InvalidPath.
#define COOKER_MAX_PATH 4096

// NOTE(saeb): Always null-terminated, since D3DCompile takes the source path as a C string.
struct CookerPath
{
    char8 Data[COOKER_MAX_PATH];
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
    usize SourceRootLength; // To print paths relative to the source root
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

static StringView8 CookerPathView(const CookerPath* path)
{
    return(StringView8{ path->Data, path->Length });
}

static void CookerPathTruncate(CookerPath* path, usize length)
{
    path->Length = length;
    path->Data[length] = '\0';
}

// NOTE(saeb): On failure (too long), the path is left partly appended; callers always truncate back to a saved length.
static bool CookerPathAppend(CookerPath* path, StringView8 text)
{
    for(usize index = 0; index < text.Length; ++index)
    {
        if(path->Length + 1 >= COOKER_MAX_PATH) // Keep room for the terminator
        {
            return(false);
        }

        path->Data[path->Length++] = text.Data[index];
    }

    path->Data[path->Length] = '\0';

    return(true);
}

static bool CookerEquals(StringView8 a, const char* b)
{
    usize index = 0;
    for(; index < a.Length; ++index)
    {
        if(b[index] == '\0' || (char)a.Data[index] != b[index])
        {
            return(false);
        }
    }

    return(b[index] == '\0');
}

// NOTE(saeb): Case-insensitive for ASCII, so "Something.PNG" still cooks. Suffixes are lowercase ASCII.
static bool CookerEndsWith(StringView8 name, const char* suffix)
{
    usize suffixLength = 0;
    while(suffix[suffixLength] != '\0')
    {
        ++suffixLength;
    }

    if(suffixLength > name.Length)
    {
        return(false);
    }

    const char8* tail = name.Data + (name.Length - suffixLength);
    for(usize index = 0; index < suffixLength; ++index)
    {
        char8 c = tail[index];
        if(c >= 'A' && c <= 'Z')
        {
            c = (char8)(c + ('a' - 'A'));
        }

        if((char)c != suffix[index])
        {
            return(false);
        }
    }

    return(true);
}

static CookerSourceType CookerGetSourceType(StringView8 name)
{
    if(CookerEndsWith(name, ".png"))
    {
        return(CookerSourceType::Texture);
    }

    if(CookerEndsWith(name, ".hlsl"))
    {
        return(CookerSourceType::Shader);
    }

    return(CookerSourceType::Unknown);
}

// NOTE(saeb): An output only counts if it was cooked with the current format; the engine rejects any other version, so an old one must be recooked even when it's newer than its source.
static bool CookerOutputIsCurrent(StringView8 outputPath)
{
    AssetHeader header = {};
    if(FileReadRange(Context.Memory, outputPath, 0, &header, sizeof(header)) != FileReadResult::Ok)
    {
        return(false);
    }

    return(header.Magic == AG_ASSET_MAGIC && header.Version == AG_ASSET_VERSION);
}

static void CookerCookFile(CookerWalk* walk, CookerSourceType type, uint64 sourceTime)
{
    StringView8 sourcePath = CookerPathView(&walk->Source);
    StringView8 outputPath = CookerPathView(&walk->Output);

    // NOTE(saeb): Up to date if the output is newer than its source and was cooked with the current format version. The timestamp check comes first, since it needs no file read. When a cooker change alters the output, bump AG_ASSET_VERSION (or cook with --force).
    FileInfo outputInfo;
    bool upToDate = !walk->Force && FileGetInfo(Context.Memory, outputPath, &outputInfo) == FileReadResult::Ok && !outputInfo.IsDirectory && outputInfo.LastWriteTime > sourceTime && CookerOutputIsCurrent(outputPath);

    if(upToDate)
    {
        ++walk->UpToDate;
        return;
    }

    // NOTE(saeb): A fresh frame on both heaps per file; whatever a cook allocates is gone before the next one, so memory use stays flat.
    Frame lowerFrame = GetFrame(Context.Memory, Heap::Lower);
    Frame upperFrame = GetFrame(Context.Memory, Heap::Upper);

    bool cooked = false;
    if(type == CookerSourceType::Texture)
    {
        cooked = CookTexture(&Context, sourcePath, outputPath);
    }
    else
    {
        cooked = CookShader(&Context, sourcePath, outputPath);
    }

    ReleaseFrame(Context.Memory, upperFrame);
    ReleaseFrame(Context.Memory, lowerFrame);

    if(cooked)
    {
        // NOTE(saeb): Relative to the source root, skipping the separator: "Sprites/Player.png".
        usize skip = walk->SourceRootLength + 1;
        printf("Cooked %.*s\n", (int)(sourcePath.Length - skip), (const char*)sourcePath.Data + skip);

        ++walk->Cooked;
    }
    else
    {
        ++walk->Failed;
    }
}

static void CookerWalkDirectory(CookerWalk* walk);

static bool CookerVisitEntry(const FileDirectoryEntry* entry, void* userData)
{
    CookerWalk* walk = (CookerWalk*)userData;
    StringView8 name = entry->Name;

    // NOTE(saeb): Skips dot-files and folders such as .gitkeep.
    if(name.Length == 0 || name.Data[0] == '.')
    {
        return(true);
    }

    bool isDirectory = entry->Info.IsDirectory;
    CookerSourceType type = isDirectory ? CookerSourceType::Unknown : CookerGetSourceType(name);
    if(!isDirectory && type == CookerSourceType::Unknown)
    {
        return(true);
    }

    usize sourceLength = walk->Source.Length;
    usize outputLength = walk->Output.Length;

    // NOTE(saeb): "/" works as a separator on every platform, Windows included.
    StringView8 separator = SV8(u8"/");
    bool pathsFit = CookerPathAppend(&walk->Source, separator) && CookerPathAppend(&walk->Source, name) && CookerPathAppend(&walk->Output, separator) && CookerPathAppend(&walk->Output, name);

    if(pathsFit && !isDirectory)
    {
        // NOTE(saeb): Replace the extension: "Player.png" -> "Player.aga". A known type guarantees there's a dot in the name.
        usize dot = walk->Output.Length;
        while(walk->Output.Data[dot - 1] != '.')
        {
            --dot;
        }

        CookerPathTruncate(&walk->Output, dot - 1);
        pathsFit = CookerPathAppend(&walk->Output, SV8(u8".aga"));
    }

    if(!pathsFit)
    {
        CookerPathTruncate(&walk->Source, sourceLength);
        CookerError(CookerPathView(&walk->Source), "path too long inside this directory");
        ++walk->Failed;
    }
    else if(isDirectory)
    {
        CookerWalkDirectory(walk);
    }
    else
    {
        CookerCookFile(walk, type, entry->Info.LastWriteTime);
    }

    CookerPathTruncate(&walk->Source, sourceLength);
    CookerPathTruncate(&walk->Output, outputLength);

    return(true);
}

static void CookerWalkDirectory(CookerWalk* walk)
{
    FileReadResult result = FileListDirectory(Context.Memory, CookerPathView(&walk->Source), CookerVisitEntry, walk);
    if(result != FileReadResult::Ok)
    {
        CookerError(CookerPathView(&walk->Source), "couldn't list directory (%s)", CookerDescribeRead(result));
        ++walk->Failed;
    }
}

int CookerRun(StackAllocator* memory, int argumentCount, const StringView8* arguments)
{
    Context.Memory = memory;

    StringView8 sourceArgument = { nullptr, 0 };
    StringView8 outputArgument = { nullptr, 0 };
    bool force = false;
    bool usage = false;

    for(int index = 0; index < argumentCount; ++index)
    {
        if(CookerEquals(arguments[index], "--debug"))
        {
            Context.Debug = true;
        }
        else if(CookerEquals(arguments[index], "--force"))
        {
            force = true;
        }
        else if(!sourceArgument.Data)
        {
            sourceArgument = arguments[index];
        }
        else if(!outputArgument.Data)
        {
            outputArgument = arguments[index];
        }
        else
        {
            usage = true; // Too many arguments
        }
    }

    if(usage || !sourceArgument.Data || !outputArgument.Data)
    {
        printf("Usage: AfterglowCooker <SourceDir> <OutputDir> [--debug] [--force]\n");
        return(1);
    }

    FileInfo sourceInfo;
    if(FileGetInfo(memory, sourceArgument, &sourceInfo) != FileReadResult::Ok || !sourceInfo.IsDirectory)
    {
        CookerError(sourceArgument, "source directory doesn't exist");
        return(1);
    }

    // NOTE(saeb): About 8 KiB with both path buffers; fine on the stack.
    CookerWalk walk = {};
    walk.Context = &Context;
    walk.Force = force;

    if(!CookerPathAppend(&walk.Source, sourceArgument) || !CookerPathAppend(&walk.Output, outputArgument))
    {
        CookerError(StringView8{ nullptr, 0 }, "source or output directory path is too long");
        return(1);
    }

    walk.SourceRootLength = walk.Source.Length;

    CookerWalkDirectory(&walk);

    printf("[Afterglow] %u cooked, %u up to date, %u failed.\n", walk.Cooked, walk.UpToDate, walk.Failed);

    return((walk.Failed > 0) ? 1 : 0);
}
