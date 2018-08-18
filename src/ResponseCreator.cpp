#include "ResponseCreator.hpp"
#include "Log.hpp"
#include "Utility.hpp"
#include "MIMERegistry.hpp"

#include <exception>
#include <fstream>
#include <vector>
#include <ctime>
#include <sys/types.h>
#include <dirent.h>
#include <string>
#include <filesystem>

namespace
{
    std::string file_extension(std::string_view location)
    {
        auto ext = location.substr(location.find_last_of('/'));
        if (auto pos = ext.find_last_of('.'); pos != std::string_view::npos)
            return std::string{ext.substr(pos + 1)};
        return {};
    }
}

namespace ryuuk
{
    using namespace std::literals::string_literals;
    namespace fs = std::filesystem;

    const std::unordered_map<ResponseCreator::StatusCode, std::string, std::hash<int>> ResponseCreator::responsePhrase = {
                    {OK,                "OK"},
                    {MovedPermanently,  "Moved Permanently"},
                    {BadRequest,        "Bad Request"},
                    {Forbidden,         "Forbidden"},
                    {NotFound,          "Not Found"},
                    {MethodNotAllowed,  "Method Not Allowed"},
                    {InternalError,     "Internal Server Error"}
    };

    const std::string ResponseCreator::serverName = "ryuuk/0.2";

    std::string_view SimpleResponse::nextChunk()
    {
        if (!end)
        {
            end = true;
            return m_responseString;
        }
        return {};
    }

    FileResponse::FileResponse(std::string&& httpPrefix, const std::string& location)
        : m_file(location)
        , m_data(std::move(httpPrefix))
        , m_responseSize(fs::file_size(fs::path{location}) + m_data.size())
    {}

    const static std::size_t ChunkMaxSize = 128 * 1024 * 1024; // 64 MB

    std::string_view FileResponse::nextChunk()
    {
        switch(m_state)
        {
            case State::Uninitialized:
            {
                std::size_t offset = m_data.size();
                m_data.resize(std::min(ChunkMaxSize, m_responseSize));
                try {
                    m_file.read(&m_data[offset], m_data.size() - offset);
                    m_transferred += m_data.size();
                } catch (const std::ios_base::failure& e) {
                    return {};
                    m_state = State::Finished;
                }

                m_state = State::Transferring;
                break;
            }
            case State::Transferring:
                try {
                    m_file.read(&m_data[0], m_data.size());
                } catch (const std::ios_base::failure& e) {
                    return {};
                    m_state = State::Finished;
                }

                if (m_file.eof())
                    m_data.resize(m_responseSize - m_transferred);

                m_transferred += m_data.size();
                break;
            case State::Finished:
                return {};
        }

        if (m_state != State::Finished && m_transferred == m_responseSize)
            m_state = State::Finished;
        return m_data;
    }

    std::string getDate()
    {
        std::string date_str;
        date_str.resize(40);    // I'm confident, the string will take exactly 30, but just to be sure...
        auto t = std::time(nullptr);
        // Date format example: Wed, 10 May 2017 06:49:35 GMT
        auto res = std::strftime(&date_str[0], date_str.size(), "%a, %d %b %Y %H:%M:%S GMT", std::gmtime(&t));
        if (res == 0)
        {
            LOG(ERROR) << "Error in generating date string" << std::endl;
        }
        date_str.resize(res);  // res is bytes written minus the null, thus we also remove the ending null
        LOG(DEBUG) << "Date: " << date_str << std::endl;
        return date_str;
    }

    std::unique_ptr<Response> ResponseCreator::create(StatusCode code, const std::string& location,
                                                      unsigned int flags)
    {
        bool directory      = ((flags & SendDirectory) == SendDirectory),
             nopayload      = ((flags & NoPayload)     == NoPayload),
             keepConnection = ((flags & KeepConnection)== KeepConnection),
             httpLegacy     = ((flags & HTTPLegacy)    == HTTPLegacy);

        // Status line
        m_responseString = "HTTP/" + (httpLegacy ? "1.0 "s : "1.1 "s) +
                            std::to_string(code) + " " + responsePhrase.at(code) + "\r\n"
                           "Server: " + serverName + "\r\n"
                           "Date: " + getDate() + "\r\n";
        if (!httpLegacy)
            m_responseString += "Connection: " + (keepConnection ? "keep-alive"s : "close"s) + "\r\n";

        LOG(INFO) << "Response: " << code << std::endl;

        // Dispatch on the status code and directory flag
        switch (code)
        {
            case OK:
                if (!directory)
                {
                    sendResource(location, nopayload);
                    if (!nopayload)
                        return std::make_unique<FileResponse>(std::move(m_responseString), location);
                }
                else
                    sendDirectoryListing(location, nopayload);
                break;
            case MovedPermanently:
                permanentRedirect(location);
                break;
            case BadRequest:
            case Forbidden:
            case NotFound:
            case MethodNotAllowed:
            case InternalError:
                sendGenericError(code, nopayload);
                break;
            default:
                // An exception will be thrown when trying to access the responsePhrase map before we ever get to this line
                throw std::invalid_argument("Status code " + std::to_string(code) + " not implemented.");
        }

        return std::make_unique<SimpleResponse>(std::move(m_responseString));
    }

    void ResponseCreator::sendResource(const std::string& location, bool nopayload)
    {
        if (auto path = fs::path{location}; fs::is_regular_file(path))
        {
            std::uintmax_t size;
            try {
                size = fs::file_size(path);
            } catch(const fs::filesystem_error& e) {
                return sendGenericError(ResponseCreator::InternalError, nopayload);
            }

            m_responseString += "Accept-Ranges: none\r\n"s
                             +  "Content-Type: "s + MIMERegistry::fromExtension(file_extension(location)) + "\r\n"
                             +  "Content-Length: "s + std::to_string(size) +
                                "\r\n\r\n";
        }
        else
            return sendGenericError(ResponseCreator::InternalError, nopayload);
    }

    void ResponseCreator::sendGenericError(StatusCode code, bool nopayload)
    {
        const std::string html = "<html><head><title>Ryuuk</title></head><body><h2>" +
                                 std::to_string(code) + " " + responsePhrase.at(code) +
                                 "</h2><hr><br><br>"
                                 "The requested resource could not be sent. Light got to this location before you, unfortunately."
                                 "<br/><br/><br/><hr>"
                                 "<i>Hosted using <a href=\"https://github.com/amhndu/ryuuk\">Ryuuk</a></i></body></html>";

        if (code == MethodNotAllowed)
                m_responseString += "Allow: GET, HEAD\r\n";
        if (code != BadRequest && code != MethodNotAllowed)
        {
            m_responseString += "Content-Type: text/html\r\n"
                                "Content-Length: " + std::to_string(html.size()) + "\r\n\r\n";
            if (!nopayload)
                m_responseString += html;
        }
        else
            m_responseString += "\r\n";
    }

    void ResponseCreator::sendDirectoryListing(const std::string& path, bool nopayload)
    {
        // Possibly read these from config or some other file ?
        static const std::string html_template = "<html>\n<head><title>Directory Listing for $DIR</title></head>\n<body>\n"
                                          "<h2>Index of $DIR</h2><hr/>\n<ul>\n$LIST</ul>\n<hr>"
                                          "<i>Hosted using <a href=\"https://github.com/amhndu/ryuuk\">Ryuuk</a></i>"
                                          "</body>\n</html>";
        static const std::string entry_template = "<li><a href=\"$URL\">$URL</a></li>\n";

        std::string html = replaceAll(html_template, "$DIR", path);
        std::vector<std::string> listing;

        DIR *dir;
        dirent *ep;
        dir = opendir(path.c_str());
        if (dir != nullptr)
        {
            m_responseString += "Accept-Ranges: none\r\n"
                                "Content-Type: text/html; charset=utf-8\r\n";

            while ((ep = readdir(dir)))
            {
                std::string res = {ep->d_name};
                if (getResourceType(path + res) == Directory)
                    res += '/';

                listing.push_back(res);
            }
            closedir(dir);

            std::sort(listing.begin(), listing.end());
            std::string listing_buf;
            for (auto&& item : listing)
                listing_buf.append(replaceAll(entry_template, "$URL", item));

            html = replaceAll(html, "$LIST", listing_buf);
            m_responseString += "Content-Length: " + std::to_string(html.size()) +
                                "\r\n\r\n";
            if (!nopayload)
                m_responseString += html;
        }
        else
        {
            LOG(ERROR) << "Couldn't open directory " << path << " to send directory listing" << std::endl;
            sendGenericError(ResponseCreator::InternalError, nopayload);
        }
    }

    void ResponseCreator::permanentRedirect(const std::string& new_location)
    {
        m_responseString += "Location: " + new_location + "\r\n\r\n";
    }
}
