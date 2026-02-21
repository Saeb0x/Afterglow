#pragma once

#include <string>
#include <memory>

namespace Afterglow
{
    struct WindowConfig
    {
        std::string Title = "Afterglow";
        uint32_t Width = 1280;
        uint32_t Height = 720;
        bool VSync = true;
        bool Fullscreen = false;
        bool Resizable = false;
    };

    class Window
    {
    public:
        virtual ~Window() = default;
        
        virtual void Update() = 0;
        virtual bool ShouldClose() const = 0;
        
        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
        
        virtual void* GetNativeHandle() const = 0;  // Platform-specific
        
        virtual void SetVSync(bool enabled) = 0;
        virtual bool IsVSync() const = 0;

        static std::unique_ptr<Window> Create(const WindowConfig& config = WindowConfig());
    };
}