#if !defined(ASSET_FORMAT_H)

#include "Core/Types.h"

struct AssetHeader
{
    char Identifier[4];
    uint32 Version;
};

#define ASSET_FORMAT_H
#endif
