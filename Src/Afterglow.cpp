// NOTE(saeb): The whole engine as one translation unit (a "unity build"): games compile this file plus their own, so the engine can add or rename files without any game's build changing. Every file here must still include everything it uses, so it would also compile on its own, and file-local names (static functions and globals, macros) must be unique across the engine; the Win32 / D3D11 / Asset prefixes keep them apart.

// Platform layer.
#include "Engine/Platform/Windows/Win32File.cpp"
#include "Engine/Platform/Windows/Win32Input.cpp"
#include "Engine/Platform/Windows/Win32Time.cpp"
#include "Engine/Platform/Windows/Win32Window.cpp"

// Renderer.
#include "Engine/Renderer/D3D11/D3D11Renderer.cpp"
#include "Engine/Renderer/Text.cpp"

// Assets.
#include "Engine/Asset/Asset.cpp"

// Entry point.
#include "Engine/Platform/Windows/Win32Main.cpp"
