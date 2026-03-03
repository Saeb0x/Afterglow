#pragma once

#include "Renderer/RendererAPI.h"

namespace Afterglow
{
	class OpenGLRendererAPI : public RendererAPI
	{
	public:
		void Init() override;
		void SetClearColor(float r, float g, float b, float a) override;
		void Clear() override;
	};
}