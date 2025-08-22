#pragma once

#include "Afterglow/Core/Layer.h"

namespace Afterglow
{
	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer() override;
	
		void OnAttach() override;
		void OnDetach() override;
		void OnEvent(Event& event) override;

		void Begin();
		void End();
	};
}

