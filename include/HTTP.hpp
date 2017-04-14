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
        enum FileType
        {
            Regular,
            Directory,
            PermissionDenied,
            NonExistent,
            Other,      // Devices, pipes, sockets etc
        };
        FileType getResourceType(const std::string& location);
        bool sendResource(const std::string& location);
        void sendInternalError();
        void sendNotFound();
        void sendPermissionDenied();
        bool sendDirectoryListing(const std::string& path);
        std::string m_response;
    };
}

#endif // HTTP_H
