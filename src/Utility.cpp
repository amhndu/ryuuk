/*

    Some general purpose methods used throughout

*/

#include <sstream>

#include "Utility.hpp"

namespace ryuuk
{
    std::string conv(const std::string& s)
    {
        std::stringstream r;
        for (auto&& c : s)
        {
            if (isprint(c))
                r << c;
            else
                r << "\\" << std::oct << std::setw(3) << std::setfill('0') << +c;
        }
        return r.str();
    }

    std::string replaceAll(const std::string& str, const std::string& key, const std::string& replacement)
    {
        std::string result {str};
        std::size_t i = 0;
        while ((i = result.find(key, i)) != std::string::npos)
        {
            result.replace(i, key.size(), replacement);
            i += replacement.size();
        }
        return result;
    }
}
