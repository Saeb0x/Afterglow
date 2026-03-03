#include "RenderCommand.h"
#include "Renderer.h"

namespace Afterglow
{
    void RenderCommand::SetClearColor(float r, float g, float b, float a)
    {
        Renderer::GetAPI().SetClearColor(r, g, b, a);
    }

    void RenderCommand::Clear()
    {
        Renderer::GetAPI().Clear();
    }
}