#ifndef RESPONSE_HPP_INCLUDED
#define RESPONSE_HPP_INCLUDED

#include <unordered_map>
#include <string>
#include <functional>
#include <iterator>
#include <string_view>
#include <memory>
#include <fstream>

namespace ryuuk
{
    class Response
    {
    public:
        virtual ~Response() {};
        virtual std::string_view nextChunk() = 0;
    };

    class SimpleResponse : public Response
    {
    public:
        SimpleResponse(std::string&& str) : m_responseString(std::move(str)) {}

        std::string_view nextChunk() override;
    private:
        std::string m_responseString;
        bool end = false;
    };

    class FileResponse : public Response
    {
    public:
        FileResponse(std::string&& httpPrefix, const std::string& location);
        std::string_view nextChunk() override;

        enum class State { Uninitialized, Transferring, Finished };
    private:
        State m_state = State::Uninitialized;
        std::ifstream m_file;
        std::string m_data;
        std::uintmax_t m_responseSize;
        std::uintmax_t m_transferred = 0;
    };

    class ResponseCreator
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

        ResponseCreator() = default;

        // Different flags can be set by OR-ing them. Like SendDirectory | NoPayload
        std::unique_ptr<Response> create(StatusCode code, const std::string& location = {}, unsigned int flags = None);
    private:
        void sendResource(const std::string& location, bool nopayload);

        void sendGenericError(StatusCode code, bool nopayload);

        void sendDirectoryListing(const std::string& path, bool nopayload);

        void permanentRedirect(const std::string& new_location);

        std::string m_responseString;

        const static std::unordered_map<StatusCode, std::string, std::hash<int>> responsePhrase;
        const static std::string serverName;
    };

}
#endif // RESPONSE_HPP_INCLUDED
