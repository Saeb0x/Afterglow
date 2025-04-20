#pragma once

namespace Afterglow
{
	class Application
	{
	public:
		Application() = default;
		virtual ~Application() = default;

		virtual void Init() = 0;
		virtual void Run();
	};
}
