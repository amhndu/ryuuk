#include <regex>

#include "HTTP.hpp"
#include "Log.hpp"

namespace ryuuk
{
    HTTP::HTTP(const std::string& request) : m_request(request)
    {

    }

    std::string HTTP::buildResponse()
    {
        std::string response;
        const std::regex request_line (R"(^([A-Z]+)[ \t]+(.+)[ \t]+HTTP/(\d\.\d))");/*(\r?\n)*/

        /* Method whitespace location whitespace HTTP/version line-end headers */
        std::smatch matches;
        if (std::regex_search(m_request, matches, request_line))
        {
            std::string method      = matches[1],
                        location    = matches[2],
                        version     = matches[3]/*,
                        line_end    = matches[4]*/;

            LOG(DEBUG) << "Parsing HTTP request line" << std::endl;
            LOG(DEBUG) << "Method: " << method << std::endl;
            LOG(DEBUG) << "Location: " << location << std::endl;
            LOG(DEBUG) << "version: " << version << std::endl;
//             if (line_end == "\r\n")
//                 LOG(DEBUG) << "CR-LF line ending" << std::endl;
//             else
//                 LOG(DEBUG) << "LF line ended" << std::endl;

            response = "HTTP/1.0 200 OK\r\n";
            std::string html = "<html><head><title>Ryuuk</title></head><body><h1>I’ll take a potato chip AND EAT IT!</h1></body>";

            response = "HTTP/1.0 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: " + std::to_string(html.size()) + "\r\n" +
            "\r\n" +
            html;
        }
        else
        {
            response = "HTTP/1.0 400 Bad Request\r\n";
            LOG(INFO) << "Malformed request received" << std::endl;
        }

        return response;

    }

}