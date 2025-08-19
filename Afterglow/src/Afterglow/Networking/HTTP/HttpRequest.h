#pragma once

#include "Afterglow/Core/Networking/NetworkingCommon.h"

namespace Afterglow
{
    namespace Networking
    {
        class HttpRequest
        {
        public:
            HttpRequest(HttpMethod method, const std::string& url)
                : m_Method(method), m_URL(url) 
            {
            }

            inline void SetHeader(const std::string& key, const std::string& value) { m_Headers[key] = value; }
            inline void SetBody(const std::string& body) { m_Body = body; }

            inline HttpMethod GetMethod() const { return m_Method; }
            inline const std::string& GetUrl() const { return m_URL; }
            inline const Headers& GetHeaders() const { return m_Headers; }
            inline const std::string& GetBody() const { return m_Body; }

        private:
            HttpMethod m_Method;
            std::string m_URL;
            Headers m_Headers;
            std::string m_Body;
        };
    }
}