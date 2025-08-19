#pragma once

#include "Afterglow/Networking/NetworkingCommon.h"

namespace Afterglow
{
	namespace Networking
	{
        class HttpResponse
        {
        public:
            HttpResponse(int status, const std::string& body)
                : m_StatusCode(status), m_Body(body) 
            {
            }

            inline int GetStatusCode() const { return m_StatusCode; }
            inline const std::string& GetBody() const { return m_Body; }

        private:
            int m_StatusCode;
            std::string m_Body;
        };
	}
}