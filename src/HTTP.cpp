#include <regex>
#include <errno.h>
#include <algorithm>
#include <vector>

#include "Log.hpp"
#include "Utility.hpp"
#include "HTTP.hpp"
#include "ResponseCreator.hpp"

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


    HTTP::Result HTTP::buildResponse(const std::string& request)
    {
        Result result;
        // Perliminary test to see if we have the entire header before regex-ing it.
        // Try to find either \n\n or \r\n\r\n
        auto end = request.find("\n\n");
        if (end != std::string::npos)
            result.bytesRead = end + 2;
        else if ((end  = request.find("\r\n\r\n")) != std::string::npos)
            result.bytesRead = end + 4;
        else
        {
            result.bytesRead = 0;
            result.keepAlive = true;  // So we keep going after this attempt
            return result;
        }


        /*
        * Part of the pattern          Remark ([x] means this is capture group number x)
        * --------------------------------------------------------------------------------------------------------------
        * ^                            Beginning of the string
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
        * ***** THIS PART'S BEEN REMOVED ******* (.|[\r\nu2029u2028])*        Body. Anything goes. [6]
        *
        * P.S. Those weird looking patterns involving u<codepoint> exist to match *anything*,
        * since '.' doesn't match those particular code points
        */
        const std::regex request_pattern (
            R"(^([A-Z]+)[ \t]+(.+)[ \t]+HTTP/(\d\.\d)(\r?\n)((?:.|[\r\nu2029u2028])*\4)*\4)"
        );

        ResponseCreator responseCreator;

        std::smatch matches;
        if (std::regex_search(request, matches, request_pattern))
        {
            std::string method      = matches[1],
                        location    = matches[2],
                        version     = matches[3],
                        line_end    = matches[4],
                        headers     = matches[5];
                        // body        = matches[6];
            LOG(INFO) << "Request line : " << method << " " << location << " HTTP/" << version << std::endl;

            // Some simple searching
            result.keepAlive = true;
            // Pattern: field-name ":" white-space field-value [CR] LF
            const std::regex fieldPattern("^([a-zA-Z\\-]+):\\s+([a-zA-Z0-9\\-\\.\\/\\(\\),\\+:; \\=\\*]+)(\r?\n)");
            auto search = headers;
            while (std::regex_search(search, matches, fieldPattern))
            {
                std::string name  = matches[1],
                            value = matches[2];


                if (name == "Connection")
                {
                    if (value == "close")
                    {
                        result.keepAlive = false;
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



            unsigned int flags = (method == "HEAD" ? ResponseCreator::NoPayload : ResponseCreator::None)
                              | (result.keepAlive ? ResponseCreator::KeepConnection : ResponseCreator::None);
            if (version == "1.0")
            {
                result.keepAlive = false;
                flags |= ResponseCreator::HTTPLegacy;
            }

            if (method != "GET" && method != "HEAD")
            {
                responseCreator.create(ResponseCreator::MethodNotAllowed, {}, flags);
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
                        result.response = responseCreator.create(ResponseCreator::OK, location, flags);
                        break;
                    case Directory:
                        // If the path doesn't have a slash, redirect by adding it, this makes relative links work properly
                        // TODO FIXME instead of sending orig_loc, send urlEncode(location.substr(1))
                        if (location.back() != '/')
                            result.response = responseCreator.create(ResponseCreator::MovedPermanently,
                                                                     orig_loc + '/', flags);
                        else
                            result.response = responseCreator.create(ResponseCreator::OK, location, ResponseCreator::SendDirectory | flags);
                        break;
                    case PermissionDenied:
                        result.response = responseCreator.create(ResponseCreator::Forbidden, {}, flags);
                        break;
                    case NonExistent:
                        result.response = responseCreator.create(ResponseCreator::NotFound, {}, flags);
                        break;
                    case Other:
                        result.response = responseCreator.create(ResponseCreator::InternalError, {}, flags);
                        break;
                }
            }
            catch (const std::domain_error& e)
            {
                LOG(INFO) << "Attempt to retrieve resource outside current directory" << std::endl;
                responseCreator.create(ResponseCreator::Forbidden, {}, flags);
            }
        }
        else
        {
            responseCreator.create(ResponseCreator::BadRequest);
        }

        return result;
   }
}
