/**
* HTTPHeader
* ----------
* An abstraction of the HTTP headers
*/

#ifndef HTTPHEADER_HPP
#define HTTPHEADER_HPP

#include <map>
#include <vector>
#include <string>

namespace ryuuk
{

    enum HeaderType
    {
        General ,
        Request ,
        Response,
        Entity
    };

    /**
    * Class HeaderElement is an abstraction
    * for a header name:value pair, e.g.,
    * `Accept: text/plain` or `Cache-Control: no-cache`
    */
    class HeaderElement
    {
    public:

        static HeaderType getHeaderType(const std::string& name);

        HeaderElement(HeaderType type, const std::string& name, const std::vector<std::string>& value)
         : m_type{type}  ,
           m_name{name}  ,
           m_value{value}
        {
        }

        // Return the header object's type
        inline const HeaderType& getType() const
        {
            return m_type;
        }

        // Return the header object's name
        // e.g., Accept, Cache-Control, etc.
        inline const std::string& getName() const
        {
            return m_name;
        }

        // Return the header object's value
        inline const std::vector<std::string>& getValue() const
        {
            return m_value;
        }

    private:

        HeaderType               m_type;
        std::string              m_name;
        std::vector<std::string> m_value;
    };


    class HTTPHeader
    {
    public:

//        enum HeaderType
//        {
//            General ,
//            Request ,
//            Response,
//            Entity
//        };

        // The header map. It is organized as follows:
        //
        // HEADER-MAP-KEY  |--------------------------- HEADER-MAP-VALUE ---------------------------|
        //       |         |                                                                        |
        //
        // [ Header-Name : [ Header-Type, { Header-Value-0, Header-Value-1, ..., Header-Value-n } ] ]
        //
        //                 |------------------------------ VALUE-MAP ------------------------------ |
        //
        // |                                                                                        |
        // |-------------------------------------- Header MAP --------------------------------------|
        //
        //
        // The format of the HTTP message header as prescribed by RFC-2616:
        //
        // message-header = field-name ":" [ field-value ]
        // field-name     = token
        // field-value    = *( field-content | LWS )
        // field-content  = <the OCTETs making up the field-value
        //                  and consisting of either *TEXT or combinations
        //                  of token, separators, and quoted-string>
        //
        //
        // NOTE:
        // Though LWS and folding are prescribed by the standard, Ryuuk currently supports
        // only in the format prescribed above. All other forms are silently ignored.
        // E.g., `Cache-Control: no-cache, no-store, must-revalidate`
        //
        static std::map<HeaderType, std::map<std::string, std::vector<std::string>>> allowed_headers;

        static void initAllowedHeaders();

    public:

        void interpretHeaders(const std::string& headers);

        void applyResponseHeaders();

    private:

        std::vector<HeaderElement> m_suppliedHeaders;
    };

}

#endif // HTTPHEADER_HPP
