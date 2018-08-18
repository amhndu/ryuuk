#ifndef HTTP_H
#define HTTP_H

#include "ResponseCreator.hpp"

#include <string>
#include <map>
#include <utility>

namespace ryuuk
{
    class HTTP
    {
    public:
        struct Result
        {
            std::unique_ptr<Response> response;
            bool keepAlive;         // Whether connection should be kept
            std::size_t bytesRead;  // Bytes consumed from the request (may not be request.size())
        };

        Result buildResponse(const std::string& request);
    private:

        std::string m_response;
    };
}

#endif // HTTP_H
