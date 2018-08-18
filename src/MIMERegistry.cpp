#include "MIMERegistry.hpp"

namespace ryuuk
{
    using namespace std::literals::string_view_literals;
    using namespace std::literals::string_literals;

    std::unordered_map<std::string, std::string> MIMERegistry::mimeTypes {{""s, "application/octet-stream"s}};

    void MIMERegistry::registerMIME(const std::string& extension, const std::string& mime)
    {
        mimeTypes.emplace(extension, mime);
    }

    std::string MIMERegistry::fromExtension(const std::string& extension)
    {
        auto it = mimeTypes.find(std::string{extension});
        return it != mimeTypes.end() ? it->second : "application/octet-stream"s;
    }

}
