#pragma once

#include <string>
#include <map>

namespace Afterglow
{
	namespace Networking
	{
		using Headers = std::map<std::string, std::string>;

		enum class HttpMethod
		{
			None = 0,
			GET,
			POST,
			PUT,
			DELETE_
		};
	}
}