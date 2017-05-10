#include "Response.hpp"
#include "Log.hpp"
#include "Utility.hpp"

#include <exception>
#include <fstream>
#include <vector>
#include <ctime>
#include <sys/types.h>
#include <dirent.h>

namespace ryuuk
{
    std::unordered_map<std::string, std::string> Response::mimeTypes{};

    const std::unordered_map<Response::StatusCode, std::string> Response::responsePhrase = {
                    {OK,                "OK"},
                    {MovedPermanently,  "Moved Permanently"},
                    {BadRequest,        "Bad Request"},
                    {Forbidden,         "Forbidden"},
                    {NotFound,          "Not Found"},
                    {MethodNotAllowed,  "Method Not Allowed"},
                    {InternalError,     "Internal Server Error"}
    };

    const std::string Response::serverName = "ryuuk/0.1";

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

    bool Response::create(StatusCode code, const std::string& location, unsigned int flags)
    {
        bool directory      = ((flags & SendDirectory) == SendDirectory),
             nopayload      = ((flags & NoPayload)     == NoPayload),
             keepConnection = ((flags & KeepConnection)== KeepConnection);

        // Status line
        // Use `at' instead of `operator[]' because the former throws an exception if the code isn't in the map
        m_responseString = "HTTP/1.1 " + std::to_string(code) + " " + responsePhrase.at(code) + "\r\n"
                           "Server: " + serverName + "\r\n"
                           "Date: " + getDate() + "\r\n"
                           "Connection: " + (keepConnection ? "keep-alive" : "close") + "\r\n";

        LOG(INFO) << "Response: " << code << std::endl;

        // Dispatch on the status code and directory flag
        // (no breaks, 'cause returns)
        switch (code)
        {
            case OK:
                if (!directory)
                    return sendResource(location, nopayload);
                else
                    return sendDirectoryListing(location, nopayload);
            case MovedPermanently:
                return permanentRedirect(location);
            case BadRequest:
            case Forbidden:
            case NotFound:
            case MethodNotAllowed:
            case InternalError:
                return sendGenericError(code, nopayload);
            default:
                // An exception will be thrown when trying to access the responsePhrase map before we ever get to this line
                throw std::invalid_argument("Status code " + std::to_string(code) + " not implemented.");
        }
    }

    bool Response::sendResource(const std::string& location, bool nopayload)
    {
        std::ifstream file(location, std::ios_base::in | std::ios_base::binary);
        if (file.is_open() && file.good())
        {
            m_responseString += "Accept-Ranges: none\r\n"
                                "Content-Type: ";

            //auto ext = location.substr(location.find_last_of('.'));
            auto ext = location.substr(location.find_last_of('/'));

            auto pos = ext.find_last_of('.');
            if (pos != std::string::npos)
                ext = ext.substr(pos + 1);
            else
                ext = {};

            auto it = mimeTypes.find(ext);
            if (it == mimeTypes.end())
                it = mimeTypes.find("bin");

            if (it != mimeTypes.end())
                m_responseString += it->second + "\r\n";
            else
                m_responseString += "application/octet-stream";  // Default

            std::string payload(std::istreambuf_iterator<char>{file}, {});
            m_responseString += "Content-Length: " + std::to_string(payload.size()) +
                                "\r\n\r\n";
            if (!nopayload)
                m_responseString += payload;  // FIXME if payload is big, this ain't gonna work
            return true;
        }
        return false;
    }

    bool Response::sendGenericError(StatusCode code, bool nopayload)
    {
        const std::string html = "<html><head><title>Ryuuk</title></head><body><h2>" +
                                 std::to_string(code) + " " + responsePhrase.at(code) +
                                 "</h2><hr><br><br>"
                                 "The requested error could not be sent. Light got to this location before you, unfortunately."
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

        return true;
    }

    bool Response::sendDirectoryListing(const std::string& path, bool nopayload)
    {
        // Possibly read these from config or some other file ?
        const std::string html_template = "<html>\n<head><title>Directory Listing for $DIR</title></head>\n<body>\n"
                                          "<h2>Index of $DIR</h2><hr/>\n<ul>\n$LIST</ul>\n<hr>"
                                          "<i>Hosted using <a href=\"https://github.com/amhndu/ryuuk\">Ryuuk</a></i>"
                                          "</body>\n</html>";
        const std::string entry_template = "<li><a href=\"$URL\">$URL</a></li>\n";

        m_responseString += "Accept-Ranges: none\r\n"
                            "Content-Type: text/html; charset=utf-8\r\n";

        std::string html = replaceAll(html_template, "$DIR", path);
        std::vector<std::string> listing;

        DIR *dir;
        dirent *ep;
        dir = opendir(path.c_str());
        if (dir != nullptr)
        {
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
            return false;
        }
        return true;
    }

    bool Response::permanentRedirect(const std::string& new_location)
    {
        m_responseString += "Location: " + new_location + "\r\n\r\n";
        return true;
    }
}
