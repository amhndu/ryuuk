#include <regex>

#include "HTTP.hpp"
#include "Log.hpp"

namespace ryuuk
{
    std::string HTTP::buildResponse(const std::string& request)
    {
        std::string response;
        const std::regex request_line (R"(^([A-Z]+)[ \t]+(.+)[ \t]+HTTP/(\d\.\d)(\r?\n))");
        /* Regex: (Method) whitespace (location) whitespace HTTP/(version) (line-end) headers */

        std::smatch matches;
        if (std::regex_search(request, matches, request_line))
        {
            std::string method      = matches[1],
                        location    = matches[2],
                        version     = matches[3],
                        line_end    = matches[4];

            LOG(DEBUG) << "Parsing HTTP request line" << std::endl;
            LOG(DEBUG) << "Method: " << method << std::endl;
            LOG(DEBUG) << "Location: " << location << std::endl;
            LOG(DEBUG) << "version: " << version << std::endl;
            if (line_end == "\r\n")
                LOG(DEBUG) << "CR-LF line ending" << std::endl;
            else
                LOG(DEBUG) << "LF line ended" << std::endl;

            if (location[0] == '/')             // Strip the leading slash
                location = location.substr(1);

            std::ifstream file(location, std::ios_base::in | std::ios_base::binary);

            // Try appending index.html (if it's a directory)
            if (!file.is_open())
            {
                location += (location.back() == '/' ? "index.html" : "./index.html");
                LOG(DEBUG) << "Trying location: " << location << std::endl;
                file.open(location, std::ios_base::in | std::ios_base::binary);
            }

            if (file.is_open())
            {
                response =  "HTTP/1.0 200 OK\r\n";
                response += "Connection: close\r\n" +
                response += "Content-Type: ";

                auto ext = location.substr(location.find_last_of('.'));

                // TODO Read MIME types from a config file
                if (ext == ".html")
                    response += "text/html\r\n";
                else if (ext == ".css")
                    response += "text/css\r\n";
                else if (ext == ".ico")
                    response += "image/x-icon\r\n";
                else
                    response += "application/octet-stream\r\n";

                std::string payload(std::istreambuf_iterator<char>(file), {});
                response += "Content-Length: " + std::to_string(payload.size());
                response += "\r\n\r\n";
                response += payload;
            }
            else
            {
                const std::string html = "<html><head><title>Ryuuk</title></head><body><h1>It's a 404</h1><h2>Light got to this location before you, unfortunately.</h2></body>";

                response = "HTTP/1.0 404 Not Found\r\n"
                           "Content-Type: text/html\r\n"
                           "Content-Length: " + std::to_string(html.size()) + "\r\n" +
                           "\r\n" +
                            html;
            }
        }
        else
        {
            response = "HTTP/1.0 400 Bad Request\r\n\r\n";
            LOG(INFO) << "Malformed request received" << std::endl;
        }

        return response;

    }

}