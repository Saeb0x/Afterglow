#if !defined(WIN32FONT_H)

#include "Core/Types.h"
#include "Core/MemoryArena.h"
#include "Engine/Text.h"

struct D3D11RendererState;

bool32 FontLoad(D3D11RendererState* renderer, MemoryArena* transient, Font* font, const char* path);

#define WIN32FONT_H
#endif
