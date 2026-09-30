#if !defined(AFTERGLOW_ASSET_H)
#define AFTERGLOW_ASSET_H

#include <SSTL/Core/Types.h>
#include <SSTL/Core/String.h>
#include <SSTL/Memory/StackAllocator.h>

#include "Engine/Renderer/Renderer2D.h"
#include "Engine/Renderer/Text.h"

enum class AssetLoadResult : uint8
{
    Ok,
    FileError, // FileRead failed: not found, access denied, out of memory, ...
    NotAnAsset, // Too small for a header, or the magic doesn't match
    WrongVersion, // Cooked with a different AG_ASSET_VERSION; recook
    WrongType, // For example, a shader passed to AssetLoadTexture
    MissingStage, // A shader without a stage this loader needs (PSMain, plus VSMain for the default pipeline)
    Corrupt, // Sizes, offsets or dimensions don't add up: truncated or damaged
    RendererFailed, // The data was valid, but the renderer couldn't create the resource (or its table is full)
    OutOfMemory // Not enough room in the allocator for the data the asset keeps (a font's glyph table)
};

// NOTE(saeb): Loads a cooked .aga texture and creates it on the GPU. Relative paths resolve against the exe's folder. On any failure, *texture is 0 (the white texture), so callers can ignore the result and still draw. The file is read into Upper heap scratch and released before returning; the allocator is left exactly as it was.
AssetLoadResult AssetLoadTexture(StackAllocator* allocator, StringView8 path, GpuTexture* texture);

// NOTE(saeb): Engine startup only: loads the cooked default quad shader (both VSMain and PSMain) into the renderer's pipeline 0. Nothing draws until this succeeds. Same memory rules as AssetLoadTexture.
AssetLoadResult AssetLoadDefaultPipeline(StackAllocator* allocator, StringView8 path);

// NOTE(saeb): Loads a cooked .aga shader and creates a quad pipeline from its PSMain, which (for now) must take the default quad vertex shader's outputs (SV_Position, TEXCOORD, COLOR). A VSMain in the file is validated but not used yet; all quads share the renderer's vertex shader. On any failure, *pipeline is 0 (the default pipeline). Same memory rules as AssetLoadTexture.
AssetLoadResult AssetLoadPipeline(StackAllocator* allocator, StringView8 path, Renderer2DPipeline* pipeline);

// NOTE(saeb): Loads a cooked .aga font: its atlas becomes a GPU texture, and its glyph table is copied into the Lower heap, where it stays for the rest of the game (take a Lower frame first to free it with a level). On any failure, *font is zeroed, which TextDraw treats as "draw nothing", and the allocator is left exactly as it was. The file itself is read into Upper heap scratch and released before returning.
AssetLoadResult AssetLoadFont(StackAllocator* allocator, StringView8 path, Font* font);

// NOTE(saeb): Destroys the font's atlas and zeroes it. The glyph table stays in the Lower heap until its frame is released.
void AssetUnloadFont(Font* font);

// NOTE(saeb): A short, lowercase reason for error messages and logs: "the file is truncated or damaged", ...
StringView8 AssetDescribeResult(AssetLoadResult result);

#endif
