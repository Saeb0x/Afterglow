// NOTE(saeb): The whole engine as one translation unit (a "unity build"): games compile this file plus their own, so the engine can add or rename files without any game's build changing.

// Platform layer.
#include "Engine/Platform/Windows/Win32Window.cpp"
#include "Engine/Platform/Windows/Win32File.cpp"
#include "Engine/Platform/Windows/Win32Time.cpp"
#include "Engine/Platform/Windows/Win32Input.cpp"
#include "Engine/Platform/Windows/Win32Log.cpp"

// Renderer.
#include "Engine/Renderer/D3D11/D3D11Gpu.cpp"
#include "Engine/Renderer/Renderer2D.cpp"
#include "Engine/Renderer/Renderer.cpp"
#include "Engine/Renderer/Text.cpp"
#include "Engine/Renderer/Camera.cpp"
#include "Engine/Renderer/Camera3D.cpp"

// UI.
#include "Engine/UI/UI.cpp"

// Assets.
#include "Engine/Asset/Asset.cpp"

// Entry point.
#include "Engine/Platform/Windows/Win32Main.cpp"
