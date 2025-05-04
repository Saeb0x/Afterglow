#include "agpch.h"
#include "EventBus.h"

namespace Afterglow
{
	EventBus::EventBus() = default;
	EventBus::~EventBus() = default;

	EventBus& Afterglow::EventBus::GetInstance()
	{
		static EventBus instance;
		return instance;
	}
}