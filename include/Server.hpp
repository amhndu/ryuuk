#ifndef SERVER_HPP
#define SERVER_HPP

#include <list>
#include <memory>

#include "Log.hpp"
#include "SocketStream.hpp"
#include "SocketListener.hpp"

namespace ryuuk
{
    
    class Server
    {
    public:
    
        Server();
        
        ~Server();
        
        void parseConfigFile();
        
        void run();
        
        bool wait();
        
        void receive();
        
        void addClient();
        
        void shutdown();
        
    public:
    
        const std::string SERVER_CONFIG_FILE = "../ryuuk.conf";
        
        struct manifest
        {
            std::string ip;
            unsigned    port;
            unsigned    backlog;
        } server_manifest;
        
    private:
    
        SocketListener m_listener;
        std::list<std::shared_ptr<SocketStream>> m_clients;
        
        bool m_running;
    };
    
}

#endif // SERVER_HPP
