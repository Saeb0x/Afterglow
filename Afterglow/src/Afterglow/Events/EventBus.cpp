#include "agpch.h"
#include "EventBus.h"

namespace Afterglow
{
	EventBus& Afterglow::EventBus::GetInstance()
	{
		static EventBus instance;
		return instance;
	}
}