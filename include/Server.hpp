#ifndef SERVER_HPP
#define SERVER_HPP

#include "Log.hpp"
#include "SocketStream.hpp"
#include "SocketListener.hpp"
#include "Response.hpp"

#include <map>
#include <list>
#include <memory>
#include <thread>

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

        void worker(SocketStream&& sock);

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
            std::unordered_map<std::string, std::string>& mime_types = Response::mimeTypes;
        } server_manifest;

    private:
        SocketListener m_listener;
        std::mutex m_queueMutex;
        std::list<int> m_cleanupQueue;
        std::map<int, std::thread> m_connections;
        bool m_running;
    };

}

#endif // SERVER_HPP
