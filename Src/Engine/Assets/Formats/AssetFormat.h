#if !defined(AFTERGLOW_ASSET_FORMAT_H)
#define AFTERGLOW_ASSET_FORMAT_H

#include "Engine/Types.h"

struct AssetHeader
{
    char Identifier[4];
    uint32 Version;
};

#endif
