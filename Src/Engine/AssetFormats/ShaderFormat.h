#if !defined(AFTERGLOW_SHADER_FORMAT_H)
#define AFTERGLOW_SHADER_FORMAT_H

#include "Engine/Types.h"
#include "AssetFormat.h"

#define SHADER_IDENTIFIER { 'A', 'G', 'S', 'H' }
#define SHADER_VERSION 1

enum ShaderStage
{
    SHADER_STAGE_VERTEX,
    SHADER_STAGE_PIXEL
};

struct ShaderFileHeader
{
    AssetHeader Header;

    uint32 Stage;
    uint32 BytecodeSize;
};

// NOTE(saeb): Followed in the file by: BytecodeSize bytes of compiled DXBC.

#endif
