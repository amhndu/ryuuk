#include <regex>
#include <errno.h>
#include <algorithm>

#include "Log.hpp"
#include "Utility.hpp"
#include "HTTP.hpp"
#include "Response.hpp"

namespace ryuuk
{

    std::vector<std::string> parseFieldValues(const std::string& value)
    {
        std::vector<std::string> vals{};
        std::string valueCopy{value};

        std::size_t pos = 0;
        while( (pos = valueCopy.find(',', pos)) != std::string::npos )
        {
            auto v = valueCopy.substr(0, pos);

            std::size_t i;

            // Remove the quality factor value
            if ( (i = v.find(';')) == std::string::npos)
                vals.push_back(ltrim(v));
            else
                vals.push_back(ltrim(v.substr(0, i)));

            valueCopy = valueCopy.substr(pos+1);
            pos++;
        }

        auto v = valueCopy.substr(0, pos);
        std::size_t i;
        if ( (i = v.find(';')) == std::string::npos)
            vals.push_back(ltrim(v));
        else
            vals.push_back(ltrim(v.substr(0, i)));

        return vals;
    }


    std::string HTTP::buildResponse(const std::string& request, Manifest& manifest)
    {
        /*
        * Part of the pattern          Remark ([x] means this is capture group number x)
        * --------------------------------------------------------------------------------------------------------------
        * ([A-Z]+)                     Method, in all caps  [1]
        * [ \t]+                       Sequence of spaces/tabs
        * (.+)                         Location, anything goes except \r, \n, U+2029 and U+2028 [2]
        * [ \t]+                       Sequence of spaces/tabs
        * HTTP/                        HTTP/
        * (\d\.\d)                     Match X.Y where X and Y are digits [3]
        * (\r?\n)                      Either \r\n or \n (we are being lenient) [4]
        * ((?:.|[\r\nu2029u2028])*\4)* Headers, Anything goes each line followed by the same line-ending as the request line [5]
        *                              (we're expecting the client be consistent with their line ending)
        *                              (replace \4 with \r?\n to be more lenient)
        * \4                           Empty line which marks the end of the header
        * (.|[\r\nu2029u2028])*        Body. Anything goes. [6]
        *
        * P.S. Those weird looking patterns involving u<codepoint> exist to match *anything*,
        * since '.' doesn't match those particular code points
        */
        const std::regex request_pattern (
            R"(([A-Z]+)[ \t]+(.+)[ \t]+HTTP/(\d\.\d)(\r?\n)((?:.|[\r\nu2029u2028])*\4)*\4(.|[\r\nu2029u2028])*)"
        );

        Response response;

        std::smatch matches;
        if (std::regex_match(request, matches, request_pattern))
        {
            std::string method      = matches[1],
                        location    = matches[2],
                        version     = matches[3],
                        line_end    = matches[4],
                        headers     = matches[5],
                        body        = matches[6];
            LOG(DEBUG) << "Request line : " << method << " " << location << " HTTP/" << version << std::endl;

            // Some simple searching
            // TODO be more strict instead of simply searching (start by adding ^ to the front)
            manifest.keepAlive = true;
            const std::regex fieldPattern("([a-zA-Z\\-]+):\\s+([a-zA-Z0-9\\-\\.\\/\\(\\),\\+:; \\=\\*]+)(\r?\n)");
            auto search = headers;
            while (std::regex_search(search, matches, fieldPattern))
            {
                std::string name  = matches[1],
                            value = matches[2];

                if (version == "1.0")
                    manifest.keepAlive = false;

                if (name == "Connection")
                {
                    if (value == "close")
                    {
                        manifest.keepAlive = false;
                    }
                    else if (value != "keep-alive")
                    {
                        LOG(INFO) << "Unrecognized value: " << value << " for field Connection" << std::endl;
                    }
                }
                else if (name == "Accept-Encoding")
                {
                    std::smatch mm;
                    if (std::regex_search(value, mm, std::regex{"(?:identity|\\*);q=0"}))
                    {
                        LOG(ERROR) << "Identity encoding not acceptable" << std::endl;
                        // TODO send 406 Not Acceptable
                    }
                }
                else
                    LOG(INFO) << "Header field ignored (" << name << ": " << value << ")" << std::endl;

                search = matches.suffix().str();
            }


            unsigned int head = method == "HEAD" ? Response::NoPayload : Response::None;
            if (method != "GET" && method != "HEAD")
            {
                response.create(Response::MethodNotAllowed);
            }
            else try
            {
                auto orig_loc = location;
                auto loc = sanitizePath(location); // can throw std::domain_error
                location = "./" + (loc != "/" ? loc : "");

                FileType type = getResourceType(location);
                // The URL "./about" is resolved to "./about/index.html" if the index exists
                // Otherwise, a directory listing is sent instead.
                if (type == Directory)
                {
                    // Check If an index.html file is present in the path,
                    type = getResourceType(location + "/index.html");
                    if (type == Regular)
                    {
                        location += "/index.html";
                        LOG(DEBUG) << "Append index.html to path" << std::endl;
                    }
                    // If no index.html is present in the path, it's a normal directory
                    else if (type == NonExistent)
                        type = Directory;
                    else
                        LOG(ERROR) << "Here's ya edge case, what do ?" << std::endl; // TODO what do ?
                }

                switch (type)
                {
                    case Regular:
                        if (!response.create(Response::OK, location, head))
                            response.create(Response::InternalError, {}, head);
                        break;
                    case Directory:
                        // If the path doesn't have a slash, redirect by adding it, this makes relative links work properly
                        // TODO FIXME instead of sending orig_loc, send urlEncode(location.substr(1))
                        if (location.back() != '/')
                            response.create(Response::MovedPermanently,
                                            /*location.substr(1)*/ orig_loc + '/', head); // Remove the preceding '.'
                        else if (!response.create(Response::OK, location, Response::SendDirectory | head))
                            response.create(Response::InternalError, {}, head);
                        break;
                    case PermissionDenied:
                        response.create(Response::Forbidden, {}, head);
                        break;
                    case NonExistent:
                        response.create(Response::NotFound, {}, head);
                        break;
                    case Other:
                        response.create(Response::InternalError, {}, head);
                        break;
                }
            }
            catch (const std::domain_error& e)
            {
                LOG(INFO) << "Attempt to retrieve resource outside current directory" << std::endl;
                response.create(Response::Forbidden, {}, head);
            }
        }
        else
        {
            response.create(Response::BadRequest);
        }

        return response;    // Implicitly converted to string
   }
}
