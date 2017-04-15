#include <regex>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>

#include "HTTP.hpp"
#include "Log.hpp"

namespace ryuuk
{
    std::map<std::string, std::string> HTTP::mimeTypes = {};
    /*
    * Sanitize path (relative to current working directory)
    * Path above the current directory results in a domain_error being raised
    * Paths starting with or w/o a slash are treated the same
    * Trailing slash is kept if present..
    */
    std::string sanitizePath(const std::string& path)
    {
        std::vector<std::string> dirs;

        // Split the path into dirs (split by '/')
        std::stringstream ss(path);
        std::string item;
        while (std::getline(ss, item, '/'))
            dirs.push_back(item);

        // Ignore . and normalize .. by "deleting" previous non-empty directory from the path.
        for (std::size_t i = 0; i < dirs.size(); ++i)
        {
            if (dirs[i] == ".")
                dirs[i] = {};
            else if (dirs[i] == "..")
            {
                dirs[i] = {};

                auto saved = i;
                do
                {
                    if (i == 0)   // Underflow, ie. going above the current working directory
                        throw std::domain_error("Path outside current directory");
                    else
                        --i;
                }
                while (dirs[i].empty());
                dirs[i] = {};
                i = saved;
            }
        }

        ss.clear();
        ss.str({});
        std::copy_if(dirs.begin(), dirs.end(), std::ostream_iterator<std::string>(ss, "/"),
                     [](const std::string& s){ return !s.empty(); });

        auto str = ss.str();
        if (!str.empty() && path.back() != '/')   // Remove the last slash if the original path didn't had it
            str.erase(str.size() - 1);
        return str;
    }

    HTTP::FileType HTTP::getResourceType(const std::string& location)
    {
        struct stat statbuf;
        // Why do all these POSIX struct and functions share a name ?
        if (stat(location.c_str(), &statbuf) != 0)
        {
            if (errno == EACCES)
                return PermissionDenied;

            // Whatever else may be the error, we might as well assume, it doesn't exist
            return NonExistent;
        }

        if(S_ISDIR(statbuf.st_mode))
            return Directory;
        else if (S_ISREG(statbuf.st_mode))
            return Regular;
        else
            return Other;
    }

    std::string HTTP::buildResponse(const std::string& request)
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
        * ((?:.|[\r\nu2029u2028])*\4)  Header, Anything goes each line followed by the same line-ending as the request line [5]
        *                              (we're expecting the client be consistent with their line ending)
        *                              (replace \4 with \r?\n to be more lenient)
        * \4                           Empty line which marks the end of the header
        * (.|[\r\nu2029u2028])*        Body. Anything goes. [6]
        */
        const std::regex request_pattern (R"(([A-Z]+)[ \t]+(.+)[ \t]+HTTP/(\d\.\d)(\r?\n)((?:.|[\r\nu2029u2028])*\4)\4(.|[\r\nu2029u2028])*)");

        // TODO Read other header fields
        // TODO Don't assume GET as the method and handle other methods.
        std::smatch matches;
        if (std::regex_match(request, matches, request_pattern))
        {
            std::string method      = matches[1],
                        location    = matches[2],
                        version     = matches[3],
                        line_end    = matches[4],
                        headers     = matches[5],
                        body        = matches[6];

            LOG(DEBUG) << "Request line parsed: " << method << " " << location << " HTTP/" << version << std::endl;

//             if (line_end == "\r\n")
//                 LOG(DEBUG) << "CR-LF line ending" << std::endl;
//             else
//                 LOG(DEBUG) << "LF line ended" << std::endl;

            try
            {
                auto loc = sanitizePath(location); // can throw std::domain_error
                location = "./" + (loc != "/" ? loc : "");
                LOG(DEBUG) << "Sanitized location: " << location << std::endl;
                FileType type = getResourceType(location);
                LOG(DEBUG) << "Resource Type: " << type << std::endl;

                // Here cases like "http://example.com/about" are handled.
                // the URL "http://example.com/about" is resolved to
                // "http://example.com/about/index.html"
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
                        if (!sendResource(location))
                            sendInternalError();
                        break;
                    case Directory:
                        if (!sendDirectoryListing(location))
                            sendInternalError();
                        break;
                    case PermissionDenied:
                        sendPermissionDenied();
                        break;
                    case NonExistent:
                        sendNotFound();
                        break;
                    case Other:
                        sendInternalError();
                        break;
                }
            }
            catch (const std::domain_error& e)
            {
                LOG(INFO) << "Attempted to retrieve resource outside current directory" << std::endl;
                sendPermissionDenied();
            }
        }
        else
        {
            m_response = "HTTP/1.0 400 Bad Request\r\n\r\n";
            LOG(INFO) << "Malformed request received" << std::endl;
        }

        return m_response;

    }

    bool HTTP::sendResource(const std::string& location)
    {
        std::ifstream file(location, std::ios_base::in | std::ios_base::binary);
        if (file.is_open() && file.good())
        {
            m_response =  "HTTP/1.0 200 OK\r\n";
            m_response += "Connection: close\r\n" +
            m_response += "Content-Type: ";

            //auto ext = location.substr(location.find_last_of('.'));
            auto ext = location.substr(location.find_last_of('/'));
            auto pos = ext.find_last_of('.');
            if (pos != std::string::npos)
                ext = ext.substr(pos + 2);
            else
                ext = {};

            // TODO Read MIME types from a config file
//            if (ext == ".html")
//                m_response += "text/html\r\n";
//            else if (ext == ".css")
//                m_response += "text/css\r\n";
//            else if (ext == ".ico")
//                m_response += "image/x-icon\r\n";
//            else
//                m_response += "application/octet-stream\r\n";

            auto it = mimeTypes.find(ext);
            if (it != mimeTypes.end())
                m_response += it->second + "\r\n";
            else
                m_response += mimeTypes.find("bin")->second + "\r\n";

            std::string payload(std::istreambuf_iterator<char>(file), {});
            m_response += "Content-Length: " + std::to_string(payload.size());
            m_response += "\r\n\r\n";
            m_response += payload;

            return true;
        }
        return false;
    }

    void HTTP::sendInternalError()
    {
        m_response = "HTTP/1.0 500 Internal Server Error\r\n\r\n";
        LOG(INFO) << "Internal server occurred" << std::endl;
    }

    void HTTP::sendNotFound()
    {
        // Possibly read this html template from config or some other file ?
        const std::string html = "<html><head><title>Ryuuk</title></head><body><h1>It's a 404</h1><h2>Light got to this location before you, unfortunately.</h2></body>";
        m_response = "HTTP/1.0 404 Not Found\r\n"
                     "Content-Type: text/html\r\n"
                     "Content-Length: " + std::to_string(html.size()) + "\r\n" +
                     "\r\n" +
                     html;
        LOG(INFO) << "Resource not found" << std::endl;
    }

    void HTTP::sendPermissionDenied()
    {
        m_response = "HTTP/1.0 403 Forbidden\r\n\r\n";
        LOG(INFO) << "Sent forbidden response" << std::endl;
    }

    bool HTTP::sendDirectoryListing(const std::string& path)
    {
        // Possibly read these from config or some other file ?
        const std::string html_template = "<html>\n<head><title>Directory Listing for DIR</title></head>\n<body>\n<h2>Index of DIR</h2><hr/>\n<ul>\nLIST</ul>\n<hr><i>Hosted using <a href=\"https://github.com/amhndu/ryuuk\">Ryuuk</a></i></body>\n</html>";
        const std::string entry_template = "<li><a href=\"URL\">NAME</a></li>\n";

        m_response =  "HTTP/1.0 200 OK\r\n";
        m_response += "Connection: close\r\n" +
        m_response += "Content-Type: text/html\r\n";

        // TODO Regex replace is overkill for this, possibly replace with something more efficient ?
        std::string html = std::regex_replace(html_template, std::regex("DIR"), path);
        std::string listing_buf;

        DIR *dir;
        dirent *ep;
        dir = opendir(path.c_str());
        if (dir != nullptr)
        {
            while (ep = readdir(dir))
            {
                std::string res = {ep->d_name};
                if (getResourceType(path + res) == Directory)
                    res += '/';

                listing_buf.append(std::regex_replace(
                                        std::regex_replace(entry_template, std::regex("NAME"), res),
                                        std::regex("URL"),
                                        res));
            }
            closedir(dir);

            html = std::regex_replace(html, std::regex("LIST"), listing_buf);
            m_response += "Content-Length: " + std::to_string(html.size());
            m_response += "\r\n\r\n";
            m_response += html;
        }
        else
        {
            LOG(ERROR) << "Couldn't open directory " << path << " to send directory listing" << std::endl;
            return false;
        }
        return true;
    }

}
