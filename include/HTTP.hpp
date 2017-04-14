#ifndef HTTP_H
#define HTTP_H

#include <string>

namespace ryuuk
{
    class HTTP
    {
    public:
        std::string buildResponse(const std::string& request);
    private:
    };
}

#endif // HTTP_H
