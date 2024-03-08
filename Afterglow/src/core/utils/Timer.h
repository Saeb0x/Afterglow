#pragma once

#include <chrono>

namespace Afterglow
{
	namespace Core
	{
		namespace Utils
		{
			class Timer
			{
			public:
				static float GetTime();
			private:
				using Clock = std::chrono::steady_clock;
				using TimePoint = std::chrono::time_point<Clock>;

				static TimePoint m_StartTime;
			};
		}
	}
}

