#ifndef UTILITY_HPP_INCLUDED
#define UTILITY_HPP_INCLUDED


#include <string>
#include <algorithm>
#include <iomanip>

namespace ryuuk
{
    // trim from start (construct new string)
    inline std::string ltrim(const std::string &str)
    {
        std::string s(str);
        s.erase(s.begin(), std::find_if_not<decltype(s.begin()), int(int)>(s.begin(), s.end(),
                std::isspace));
        return s;
    }

    // trim from end (construct new string)
    inline std::string rtrim(const std::string &str)
    {
        std::string s(str);
        s.erase(std::find_if_not<decltype(s.rbegin()), int(int)>(s.rbegin(), s.rend(),
                std::isspace).base(), s.end());
        return s;
    }

    std::string conv(const std::string& s);

    std::string replaceAll(const std::string& str, const std::string& key, const std::string& replacement);
}


#endif // UTILITY_HPP_INCLUDED
