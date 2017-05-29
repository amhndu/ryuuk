#ifndef RESPONSE_HPP_INCLUDED
#define RESPONSE_HPP_INCLUDED

#include <unordered_map>
#include <string>
#include <functional>

namespace ryuuk
{
    class Response
    {
    public:
        enum StatusCode
        {
            // 2xx
            OK                  = 200,
            // 3xx
            MovedPermanently    = 301,
            // 4xx
            BadRequest          = 400,
            Forbidden           = 403,
            NotFound            = 404,
            MethodNotAllowed    = 405,
            // 5xx
            InternalError       = 500,
        };

        enum Flags
        {
            None            = 0,
            SendDirectory   = 1 << 1,
            NoPayload       = 1 << 2,
            KeepConnection  = 1 << 3,
            HTTPLegacy      = 1 << 4,
        };

        Response() = default;

        // Different flags can be set by OR-ing them. Like SendDirectory | NoPayload
        bool create(StatusCode code, const std::string& location = "", unsigned int flags = None);

        // Get the response string through a conversion to string
        operator std::string() { return m_responseString; };

        static std::unordered_map<std::string, std::string> mimeTypes;
    private:
        bool sendResource(const std::string& location, bool nopayload);

        bool sendGenericError(StatusCode code, bool nopayload);

        bool sendDirectoryListing(const std::string& path, bool nopayload);

        bool permanentRedirect(const std::string& new_location);

        std::string m_responseString;
        const static std::unordered_map<StatusCode, std::string, std::hash<int>> responsePhrase;
        const static std::string serverName;
    };

}
#endif // RESPONSE_HPP_INCLUDED
