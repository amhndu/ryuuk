#include <utility>
#include <regex>

#include "Log.hpp"
#include "Utility.hpp"
#include "HTTP.hpp"
#include "HTTPHeader.hpp"

namespace ryuuk
{

    inline const std::string headerToString(HeaderType headerType)
    {
        switch (headerType)
        {
            case General  : return "General" ;
            case Request  : return "Request" ;
            case Response : return "Response";
            case Entity   : return "Entity"  ;
        }
    }

    HeaderType HeaderElement::getHeaderType(const std::string& name)
    {
        // General header fields
        if (name == "Cache-Control" || name == "Connection" ||
            name == "Date" || name == "Pragma" ||
            name == "Trailer" || name == "Transfer-Encoding" ||
            name == "Via" || name == "Warning")
        return General;


        // Request header fields
        if (name == "Accept" || name == "Accept-Charset" ||
            name == "Accept-Encoding" || name == "Accept-Language" ||
            name == "From" || name == "Host" ||
            name == "If-Modified-Since" || name == "If-Unmodified-Since" ||
            name == "TE" || name == "User-Agent")
        return Request;


        // Response header fields
        if (name == "Age" || name == "Location" ||
            name == "Retry-After" || name == "Server")
        return Response;


        // Entity header fields
        if (name == "Allow" || name == "Content-Encoding" ||
            name == "Content-Language" || name == "Content-Length" ||
            name == "Content-Location" || name == "Expires" ||
            name == "Last-Modified")
        return Entity;
    }

    // The allowed headers container
    std::map<HeaderType, std::map<std::string, std::vector<std::string>>>
    HTTPHeader::allowed_headers = {};

    // Initialize the allowed headers container with
    // supported & expected values.
    void HTTPHeader::initAllowedHeaders()
    {
        LOG(DEBUG) << "Populating allowed headers list..." << std::endl;

        // Populate the supported headers

        allowed_headers[General] = {};
        allowed_headers[Request] = {};
        allowed_headers[Response] = {};
        allowed_headers[Entity] = {};

        // The general header fields and their directives
        std::pair<std::string, std::vector<std::string>>
            cacheControl,
            connection,
            date,
            pragma,
            trailer,
            transferEncoding,
            //upgrade,
            via,
            warning;

        // Anything in CAPS contained in a tag (e.g. <SOMETHING>)
        // will be replaced with actual values while building
        //the actual HTTP response.

        cacheControl = std::make_pair(std::string{"Cache-Control"},
            std::vector<std::string>{
                // Request Cache-Control directives
                "max-age=<SECONDS>" ,
                "max-stale=<SECONDS>",
                "min-fresh=<SECONDS>",
                "no-cache",
                "no-store",
                "no-transform",
                "only-if-cached"

                // Response Cache-Control directives
                "must-revalidate",
                "no-cache",
                "no-store",
                "no-transform",
                "public",
                "private",
                "proxy-revalidate",
                "max-age=<SECONDS>",
                "s-maxage=<SECONDS>"
            });

        connection = std::make_pair(std::string{"Connection"},
            std::vector<std::string>{
                "keep-alive",
                "close"
            });

        date = std::make_pair(std::string{"Date"},
            std::vector<std::string>{
                "<DAYNAME>, <DAY> <MONTH> <YEAR> <HOUR>:<MINUTE>:<SECOND> GMT"
            });

        pragma = std::make_pair(std::string{"Pragma"},
            std::vector<std::string>{
                "no-cache"
            });

        trailer = std::make_pair(std::string{"Trailer"},
            std::vector<std::string>{
                // From https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Trailer

                /*

                HTTP header fields which will be present in the trailer part of chunked messages.

                These header fields are disallowed:
                * message framing headers (e.g., Transfer-Encoding and Content-Length),
                * routing headers (e.g., Host),
                * request modifiers (e.g., controls and conditionals, like Cache-Control, Max-Forwards, or TE),
                * authentication headers (e.g., Authorization or Set-Cookie),
                  or Content-Encoding, Content-Type, Content-Range, and Trailer itself.

                */

                "<HEADER-NAME>"
            });

        transferEncoding = std::make_pair(std::string{"Transfer-Encoding"},
            std::vector<std::string>{
                "chunked",
                "compress",
                "deflate",
                "gzip",
                "identity"
            });

        via = std::make_pair(std::string{"Via"},
            std::vector<std::string>{
                "<PROTOCOL-NAME>",
                "<PROTOCOL-VERSION>",
                "<HOST>:<PORT>",
                "<PSEUDONYM>"
            });

        warning = std::make_pair(std::string{"Warning"},
            std::vector<std::string>{
                "<WARNING-CODE>",
                "<WARNING-AGENT>",
                "<WARNING-TEXT>",
                // The warning date
                "<DAYNAME>, <DAY> <MONTH> <YEAR> <HOUR>:<MINUTE>:<SECOND> GMT"
            });

        // Add the general headers to a map, and that map to the map of
        // supported headers

        std::map<std::string, std::vector<std::string>> generalMap;

        generalMap.insert(cacheControl);
        generalMap.insert(connection);
        generalMap.insert(date);
        generalMap.insert(pragma);
        generalMap.insert(trailer);
        generalMap.insert(transferEncoding);
        generalMap.insert(via);
        generalMap.insert(warning);

        allowed_headers.at(General) = generalMap;

        // The request headers

        std::pair<std::string, std::vector<std::string>>
            accept            ,
            acceptCharset     ,
            acceptEncoding    ,
            acceptLanguage    ,
            //authorization     ,
            //expect            ,
            from              ,
            host              ,
            //ifMatch           ,
            ifModifiedSince   ,
            //ifNoneMatch       ,
            //ifRange           ,
            ifUnmodifiedSince ,
            //maxForwards       ,
            //proxyAuthorization,
            //range             ,
            //referer           ,
            TE                ,
            userAgent;


        // Get the supported MIME types
        std::vector<std::string> types;
        for (auto&& x : HTTP::mimeTypes)
            types.push_back(x.second);

        // q-factor weighting ignored
        accept = std::make_pair(std::string{"Accept"}, types);

        acceptCharset = std::make_pair(std::string{"Accept-Charset"},
            std::vector<std::string>{
                "iso-8859-1",
                "iso-8859-2",
                "iso-8859-3",
                "iso-8859-4",
                "iso-8859-5",
                "iso-8859-6",
                "iso-8859-7",
                "iso-8859-8",
                "utf-8" // We really care about this only
        });

        acceptEncoding = std::make_pair("Accept-Encoding",
            std::vector<std::string>{
                "gzip",
                "compress",
                "deflate",
                "br",
                "identity",
                "*"
        });

        acceptLanguage = std::make_pair("Accept-Language",
            std::vector<std::string>{
                // For now, we only understand US English
                "en",
                "en-US",
                "*"
        });

        from = std::make_pair("From",
            std::vector<std::string>{
                "<EMAIL>"
        });

        host = std::make_pair("Host",
            std::vector<std::string>{
                "<HOSTNAME>:<PORT>"
        });

        ifModifiedSince = std::make_pair("If-Modified-Since",
            std::vector<std::string>{
                // the date
                "<DAYNAME>, <DAY> <MONTH> <YEAR> <HOUR>:<MINUTE>:<SECOND> GMT"
        });

        ifUnmodifiedSince = std::make_pair("If-Unmodified-Since",
            std::vector<std::string>{
                // the date
                "<DAYNAME>, <DAY> <MONTH> <YEAR> <HOUR>:<MINUTE>:<SECOND> GMT"
        });

        TE = std::make_pair("TE",
            std::vector<std::string>{
                "compress",
                "deflate",
                "gzip",
                "trailers"
        });

        // https://techblog.willshouse.com/2012/01/03/most-common-user-agents/
        userAgent = std::make_pair("User-Agent",
            std::vector<std::string>{
                "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:52.0) Gecko/20100101 Firefox/52.0" // my UA
                "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/57.0.2987.133 Safari/537.36",
                "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/56.0.2924.87 Safari/537.36",
                "Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/57.0.2987.133 Safari/537.36",
                "Mozilla/5.0 (Windows NT 10.0; WOW64; rv:52.0) Gecko/20100101 Firefox/52.0",
                "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_12_4) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/57.0.2987.133 Safari/537.36",
                "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_12_4) AppleWebKit/603.1.30 (KHTML, like Gecko) Version/10.1 Safari/603.1.30",
                "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:52.0) Gecko/20100101 Firefox/52.0"
        });

        std::map<std::string, std::vector<std::string>> requestMap;
        requestMap.insert(accept);
        requestMap.insert(acceptCharset);
        requestMap.insert(acceptEncoding);
        requestMap.insert(acceptLanguage);
        requestMap.insert(from);
        requestMap.insert(host);
        requestMap.insert(ifModifiedSince);
        requestMap.insert(ifUnmodifiedSince);
        requestMap.insert(TE);
        requestMap.insert(userAgent);

        allowed_headers.at(Request) = requestMap;


        std::pair<std::string, std::vector<std::string>>
            age,
            location,
            retryAfter,
            server;

        age = std::make_pair(std::string{"Age"},
            std::vector<std::string>{
                "<SECONDS>"
        });

        location = std::make_pair(std::string{"Location"},
            std::vector<std::string>{
                "<LOCATION>"
        });

        retryAfter = std::make_pair(std::string{"Retry-After"},
            std::vector<std::string>{
                "<DAYNAME>, <DAY> <MONTH> <YEAR> <HOUR>:<MINUTE>:<SECOND> GMT",
                "<SECONDS>"
        });

        server = std::make_pair(std::string{"Server"},
            std::vector<std::string>{
                "Ryuuk/0.0.0 (Gahnu/Loonigz)"
        });

        std::map<std::string, std::vector<std::string>> responseMap;
        responseMap.insert(age);
        responseMap.insert(location);
        responseMap.insert(retryAfter);
        responseMap.insert(server);

        allowed_headers[Response] = responseMap;

        std::pair<std::string, std::vector<std::string>>
            allow,
            contentEncoding,
            contentLanguage,
            contentLength,
            contentLocation,
            //contentType;
            expires,
            lastModified;

        allow = std::make_pair(std::string{"Allow"},
            std::vector<std::string>{
                "GET", "POST", "HEAD", "PUT", "DELETE", "OPTIONS", "CONNECT"
        });

        contentEncoding = std::make_pair(std::string{"Content-Encoding"},
            std::vector<std::string>{
                "gzip", "compress", "deflate", "identity", "br"
        });

        contentLanguage = std::make_pair(std::string{"Content-Language"},
            std::vector<std::string>{
                "en-US" // for now only US English is supported
        });

        contentLength = std::make_pair(std::string{"Content-Length"},
            std::vector<std::string>{
                "<BYTES>"
        });

        contentLocation = std::make_pair(std::string{"Content-Location"},
            std::vector<std::string>{
                "<RELATIVELOCATION>"
        });

//        contentType = std::make_pair(std::string{"Content-Type"},
//            std::vector<std::string>{
//                "text/html; charset=utf-8"
//        });

        expires = std::make_pair(std::string{"Expires"},
            std::vector<std::string>{
                "<DAYNAME>, <DAY> <MONTH> <YEAR> <HOUR>:<MINUTE>:<SECOND> GMT"
        });

        lastModified = std::make_pair(std::string{"Last-Modified"},
            std::vector<std::string>{
                "<DAYNAME>, <DAY> <MONTH> <YEAR> <HOUR>:<MINUTE>:<SECOND> GMT"
        });

        std::map<std::string, std::vector<std::string>> entityMap;
        entityMap.insert(allow);
        entityMap.insert(contentEncoding);
        entityMap.insert(contentLanguage);
        entityMap.insert(contentLength);
        entityMap.insert(contentLocation);
        entityMap.insert(expires);
        entityMap.insert(lastModified);

        allowed_headers[Entity] = entityMap;

        LOG(DEBUG) << "Initialized all allowed headers" << std::endl;
    }

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

    void HTTPHeader::interpretHeaders(const std::string& headers)
    {
        // We have to parse the headers which are in the form:
        // <name>:<space><value>
        //
        // Regex for matching them:
        //
        // ([a-zA-Z\-]+):\s([a-zA-Z0-9\-\.\/\(\),\+:; \=\*]+)
        //
        // Capture group [0] captures the `name`
        // Capture group [1] captures the `value`

        LOG(DEBUG) << "Interpreting the header fields..." << std::endl;

        const std::regex headerPattern("([a-zA-Z\\-]+):\\s([a-zA-Z0-9\\-\\.\\/\\(\\),\\+:; \\=\\*]+)(\r?\n)");
        // This commented out regex ain't working.
        //const std::regex headerPattern(R"([a-zA-Z\-]+):\s([a-zA-Z0-9\-\.\/\(\),\+:; \=\*]+)(\r?\n)");

        std::smatch matches;

        auto search = headers;

        LOG(DEBUG) << "Parsed header fields:-" << std::endl;

        while (std::regex_search(search, matches, headerPattern))
        {
            std::string name = matches[1],
                        value = matches[2];

            LOG(DEBUG) << "Name: " << name << ", Value: " << value << std::endl;

            // Skip certain headers
            if (name != "User-Agent" && name != "Server")
                m_suppliedHeaders.push_back(HeaderElement(HeaderElement::getHeaderType(name), name, parseFieldValues(value)));

//            for (auto&& x : m_suppliedHeaders)
//            {
//                std::cout << "Type: " << x.getType() << std::endl
//                          << "Name: " << x.getName() << std::endl;
//
//                for (auto&& y : x.getValue())
//                    std::cout << "Value: " << y << std::endl;
//            }

//            if (name != "User-Agent" && name != "Server")
//            {
//                std::cout << "Name: " << name << std::endl;
//            for (auto&& x : parseFieldValues(value))
//                std::cout << "Value: " << x << std::endl;
//            }

            search = matches.suffix().str();
        }

//         LOG(DEBUG) << "Interpreted all supplied headers" << std::endl;
        LOG(DEBUG) << "All headers ignored" << std::endl;
    }

    void HTTPHeader::applyResponseHeaders()
    {
    }

}
