#pragma once

namespace Afterglow
{
	class RenderCommand
	{
	public:
		static void SetClearColor(float r, float g, float b, float a);
		static void Clear();
	};
}