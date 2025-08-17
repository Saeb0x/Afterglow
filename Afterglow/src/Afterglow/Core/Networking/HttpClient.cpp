#include "agpch.h"
#include "HttpClient.h"

#include <curl/curl.h>

namespace Afterglow
{
	namespace Networking
	{
		static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
		{
			std::string* s = static_cast<std::string*>(userp);
			s->append((char*)contents, size * nmemb);
			return size * nmemb;
		}

		HttpResponse HttpClient::Send(const HttpRequest& request)
		{
			CURL* curl = curl_easy_init();
			std::string responseBody;
			long httpCode = 0;

            if (curl)
            {
                curl_easy_setopt(curl, CURLOPT_URL, request.GetUrl().c_str());
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);

                curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
                curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 5000L);
                curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 3000L);
                curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

                struct curl_slist* headers = nullptr;
                for (const auto& [key, value] : request.GetHeaders())
                {
                    std::string header = key + ": " + value;
                    headers = curl_slist_append(headers, header.c_str());
                }
                if (headers) curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

                if (request.GetMethod() == HttpMethod::POST)
                {
                    curl_easy_setopt(curl, CURLOPT_POST, 1L);
                    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.GetBody().c_str());
                }

                curl_easy_perform(curl);
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

                if (headers) curl_slist_free_all(headers);
                curl_easy_cleanup(curl);
            }

            return HttpResponse(static_cast<int>(httpCode), responseBody);
		}
	}
}
