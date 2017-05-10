/*

    Some general purpose methods used throughout

*/

#include <sstream>
#include <iterator>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>

#include "Utility.hpp"
#include "Log.hpp"

namespace ryuuk
{
    FileType getResourceType(const std::string& location)
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
            else // URL decode
            {
                std::size_t j = 0;
                //auto x = dirs[i].find('d');
                //auto y = dirs[i].find('%');
                while ((j = dirs[i].find('%', j)) != std::string::npos)
                {
                    if (j + 2 >= dirs[i].size())
                    {
                        LOG(INFO) << "Malformed URL" << std::endl;
                        throw std::domain_error("URL decoding error");
                    }

                    const std::string hex_ciphers{"0123456789abcdef"};
                    char c = 0;
                    for (std::size_t k = j + 1; k <= j + 2; ++k)
                    {
                        auto p = hex_ciphers.find(dirs[i][k]);
                        if (p != std::string::npos)
                            c = c * 16 + p;
                        else
                            throw std::domain_error("URL decoding error");
                    }

                    dirs[i].replace(j, 3, &c, 1);
                }
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
