#ifndef HTTP_H
#define HTTP_H

#include <string>

namespace ryuuk
{
    class HTTP
    {
    public:
        HTTP(const std::string& request);
        std::string buildResponse();
    private:
        const std::string& m_request;
    };
}

#endif // HTTP_H
