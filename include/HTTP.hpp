#ifndef HTTP_H
#define HTTP_H

#include <string>
#include <map>

#include "HTTPHeader.hpp"

namespace ryuuk
{
    class HTTP
    {
    public:

        static std::map<std::string, std::string> mimeTypes;

        std::string buildResponse(const std::string& request);

    private:

        enum FileType
        {
            Regular,
            Directory,
            PermissionDenied,
            NonExistent,
            Other,      // Devices, pipes, sockets etc
        };

        const static std::string serverName;

        FileType getResourceType(const std::string& location);

        bool sendResource(const std::string& location);

        void sendInternalError();

        void sendNotFound();

        void sendPermissionDenied();

        bool sendDirectoryListing(const std::string& path);

        void permanentRedirect(const std::string& new_location);

        void methodNotImplemented();

        HTTPHeader m_headerFields;

        std::string m_response;
    };
}

#endif // HTTP_H
