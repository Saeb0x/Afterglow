#if !defined(AFTERGLOW_WIN32FONT_H)
#define AFTERGLOW_WIN32FONT_H

#include "Engine/Types.h"
#include "SSTL/Memory.h"
#include "Engine/Text.h"

struct D3D11RendererState;

bool32 FontLoad(D3D11RendererState* renderer, sstl::Arena* transient, Font* font, const char* path);

#endif
