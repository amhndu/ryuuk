#ifndef HTTP_H
#define HTTP_H

#include <string>
#include <map>

namespace ryuuk
{
    class HTTP
    {
    public:
        // This struct is used to pass flagssettings/etc/ to the Server class.
        // I couldn't really think of a better name for the struct :p
        // Hint for future: Use this for sending chunks of data for big files
        // Also possibly signal if the received data is incomplete and need to recv again ro smthin'
        struct Manifest
        {
            bool keepAlive;     // Whether connection should be kept
            // Add other things here when applicable
        };

        std::string buildResponse(const std::string& request, Manifest& manifest);
    private:

        std::string m_response;
    };
}

#endif // HTTP_H
