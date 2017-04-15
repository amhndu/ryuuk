#ifndef SERVER_HPP
#define SERVER_HPP

#include "Log.hpp"
#include "SocketStream.hpp"
#include "SocketListener.hpp"
#include "SocketSelector.hpp"
#include "HTTP.hpp"

#include <list>
#include <map>
#include <memory>

namespace ryuuk
{

    class Server
    {
    public:

        Server();

        ~Server();

        void init();

        void parseConfigFile();

        void run();

        void receive();

        void addClient();

        void shutdown();

        void setConfigFile(const std::string& file);

    public:

        const std::string SERVER_CONFIG_FILE = "ryuuk.conf";
        std::string m_configFile;

        struct manifest
        {
            std::string ip;
            unsigned    port;
            unsigned    backlog;
            std::map<std::string, std::string>& mime_types = HTTP::mimeTypes;
        } server_manifest;

    private:

        SocketListener m_listener;
        std::list<SocketStream> m_clients;
        SocketSelector m_selector;
        bool m_running;
    };

}

#endif // SERVER_HPP
