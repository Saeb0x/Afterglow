#pragma once

#include "Afterglow/Core/Input.h"

namespace Afterglow
{
	class WindowsInput : public Input 
	{
	protected:
		bool IsKeyPressedImpl(int keyCode) const override;
		bool IsMouseButtonPressedImpl(int mouseButton) const override;
		std::pair<float, float> GetMousePositionImpl() const override;
		float GetMouseXImpl() const override;
		float GetMouseYImpl() const override;
	};
}

