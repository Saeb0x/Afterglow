#if !defined(AFTERGLOW_ASSETFORMAT_H)
#define AFTERGLOW_ASSETFORMAT_H

#include <SSTL/Core/Types.h>
#include <SSTL/Core/Assert.h>

// NOTE(saeb): The .aga (Afterglow asset) file format, shared by the engine and cooker so the two can never disagree. Little-endian only. The layout is AssetHeader, then a type-specific header, then the data. Every struct is a multiple of 16 bytes, so the data after them stays 16-byte aligned in FileRead's buffer and can be used in place.

#define AG_ASSET_MAGIC ((uint32)'A' | ((uint32)'G' << 8) | ((uint32)'A' << 16)) // "AGA\0" when viewed in a hex editor
#define AG_ASSET_VERSION 4 // Bump on any change to this file; the engine rejects other versions, so everything gets recooked
#define AG_ASSET_ALIGNMENT 16 // Alignment of every blob inside a payload
#define AG_ASSET_MAX_TEXTURE_DIMENSION 16384 // D3D11's limit; the cooker refuses larger images, the loader rejects them

enum class AssetType : uint32
{
    Unknown = 0, // A zeroed or truncated header never looks valid
    Texture,
    Shader,
    Font
};

struct AssetHeader
{
    uint32 Magic; // AG_ASSET_MAGIC
    uint32 Version; // AG_ASSET_VERSION
    AssetType Type;
    uint32 Reserved;
    uint64 PayloadSize; // Bytes after this header; must match the file size exactly
    uint64 Reserved2;
};
SSTL_ASSERT_STATIC_MSG(sizeof(AssetHeader) == 32, "Afterglow: AssetHeader must stay 32 bytes.");

// NOTE(saeb): Payload for AssetType::Texture: this header, then Width * Height * 4 bytes of RGBA8, premultiplied by the cooker, rows top to bottom.
struct AssetTextureHeader
{
    uint32 Width;
    uint32 Height;
    uint32 Reserved[2];
};
SSTL_ASSERT_STATIC_MSG(sizeof(AssetTextureHeader) == 16, "Afterglow: AssetTextureHeader must stay 16 bytes.");

// NOTE(saeb): Payload for AssetType::Shader: this header, then the compiled bytecode blobs. Offsets are from the start of the payload and AG_ASSET_ALIGNMENT-aligned; a size of 0 means the source had no such entry point (VSMain / PSMain).
struct AssetShaderHeader
{
    uint32 VertexOffset;
    uint32 VertexSize;
    uint32 PixelOffset;
    uint32 PixelSize;
};
SSTL_ASSERT_STATIC_MSG(sizeof(AssetShaderHeader) == 16, "Afterglow: AssetShaderHeader must stay 16 bytes.");

#define AG_ASSET_MAX_FONT_GLYPHS 4096 // Far more than any font is cooked with; bounds how big a glyph table the loader accepts

// NOTE(saeb): Payload for AssetType::Font: this header, then GlyphCount AssetGlyph entries sorted by codepoint, then AtlasWidth * AtlasHeight bytes of signed distance, rows top to bottom (128 is a glyph's edge, higher is inside). Sizes and positions are in pixels at PixelHeight, the size the atlas was generated at.
struct AssetFontHeader
{
    uint32 GlyphCount;
    uint32 AtlasWidth;
    uint32 AtlasHeight;
    real32 PixelHeight;
    real32 Ascent; // Above the baseline, positive
    real32 Descent; // Below the baseline, positive
    real32 LineHeight; // Baseline to baseline
    real32 DistanceRange; // Pixels from the edge to where the stored distance reaches 0 or 255
    uint32 Reserved[4];
};
SSTL_ASSERT_STATIC_MSG(sizeof(AssetFontHeader) == 48, "Afterglow: AssetFontHeader must stay 48 bytes.");

struct AssetGlyph
{
    uint32 Codepoint;
    uint16 AtlasX, AtlasY, Width, Height; // The glyph's rectangle in the atlas; 0 x 0 for glyphs with no ink, such as space
    real32 OffsetX, OffsetY; // From the pen on the baseline to the rectangle's top-left
    real32 Advance; // How far the pen moves after this glyph
    uint32 Reserved[2];
};
SSTL_ASSERT_STATIC_MSG(sizeof(AssetGlyph) == 32, "Afterglow: AssetGlyph must stay 32 bytes.");

#endif
