#ifndef SERVER_HPP
#define SERVER_HPP

#include "Log.hpp"
#include "SocketStream.hpp"
#include "SocketListener.hpp"
#include "SocketSelector.hpp"

#include <list>
#include <memory>

namespace ryuuk
{

    class Server
    {
    public:

        Server();

        ~Server();

        void parseConfigFile();

        void run();

        void receive();

        void addClient();

        void shutdown();

    public:

        const std::string SERVER_CONFIG_FILE = "ryuuk.conf";

        struct manifest
        {
            std::string ip;
            unsigned    port;
            unsigned    backlog;
        } server_manifest;

    private:

        SocketListener m_listener;
        std::list<SocketStream> m_clients;
        SocketSelector m_selector;
        bool m_running;
    };

}

#endif // SERVER_HPP
