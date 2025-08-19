#pragma once

#include "HttpRequest.h"
#include "HttpResponse.h"

namespace Afterglow
{
	namespace Networking
	{
		class HttpClient
		{
		public:
			HttpResponse Send(const HttpRequest& request);
		};
	}
}


