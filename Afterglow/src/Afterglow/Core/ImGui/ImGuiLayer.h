#pragma once

#include "Afterglow/Core/Layer.h"

namespace Afterglow
{
	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer()
			: Layer("ImGuiLayer")
		{
		}

		~ImGuiLayer()
		{
		}

		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate() override;
		void OnEvent(Event& event) override;

	private:
		void Begin();
		void End();
	};
}

