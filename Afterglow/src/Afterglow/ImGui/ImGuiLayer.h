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

		void SetConsumeEvents(bool consume) { b_ConsumeEvents = consume; }

	private:
		void SetupImGuiStyle();

	private:
		bool b_ConsumeEvents = true;
	};
}