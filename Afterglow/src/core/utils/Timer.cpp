#include "Timer.h"

namespace Afterglow
{
	namespace Core
	{
		namespace Utils
		{

			Timer::TimePoint Timer::m_StartTime = Clock::now();

			float Timer::GetTime()
			{
				auto currentTime = Clock::now();
				std::chrono::duration<float> duration = currentTime - m_StartTime;
				return duration.count();
			}
		}
	}
}