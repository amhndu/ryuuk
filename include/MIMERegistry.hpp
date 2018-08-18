#ifndef MIMEREGISTRY_H
#define MIMEREGISTRY_H
#include <unordered_map>
#include <string>

namespace ryuuk
{
    class MIMERegistry
    {
    public:
        static void registerMIME(const std::string& extension, const std::string& mime);
        static std::string fromExtension(const std::string& extension);
    private:
        static std::unordered_map<std::string, std::string> mimeTypes;
    };
}

#endif // MIMEREGISTRY_H
