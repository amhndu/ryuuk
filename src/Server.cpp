#include <fstream>

#include "Server.hpp"

namespace ryuuk
{
    
    Server::Server() : m_listener() ,
                       m_clients()  ,
                       m_running(true)
    {
        LOG(INFO) << "Hello Light. Server object created." << std::endl;
        
        LOG(INFO) << "Parsing server configuration file..." << std::endl;
        parseConfigFile();
        
        LOG(INFO) << "Attempting to bind listener (SocketListener object)..." << std::endl;        
        if (m_listener.listen(server_manifest.port, server_manifest.backlog))
            LOG(INFO) << "Successfully bound listener on port \'" + std::to_string(server_manifest.port) + "\'." << std::endl;
        else
        {
            LOG(ERROR) << "[ FATAL ] Server could not bind listener on port \'"
                       << std::to_string(server_manifest.port) << "\'. Exiting..." << std::endl;
            exit(EXIT_FAILURE);
        }
        
        m_running = true;
        
        LOG(INFO) << "Listening for incoming connections..." << std::endl;
    }
    
    Server::~Server()
    {
        //m_listener.~SocketListener(); : Undefined behaviour! ouch!
        LOG(INFO) << "Server object destroyed." << std::endl;
    }
    
    void Server::parseConfigFile()
    {
        std::ifstream configFile(SERVER_CONFIG_FILE, std::ios::in);
        
        if (!configFile.good() || !configFile.is_open())
        {
            LOG(ERROR) << "[ FATAL ] Ryuuk config file error: Unable to read config file! Terminating..." << std::endl;
            throw std::runtime_error("[ FATAL ] \'" + SERVER_CONFIG_FILE + "\': No such file exists or it is corrupted!");
        }
        
        // TODO: Create a proper config file
        
        // Read config options...
        
        // The manifest will be read from the config file when one is decided upon and ready!
        server_manifest.ip = "127.0.0.1";
        server_manifest.port = 6666;
        server_manifest.backlog = 10;
        
        configFile.close();
        
        LOG(INFO) << "Parsed and applied server configuration from \'" + SERVER_CONFIG_FILE << "\'." << std::endl;
    }
    
    void Server::run()
    {
        LOG(INFO) << "foo" << std::endl;
        while(m_running)
        {
            // Can't use SIGTERM if this line is commented out!! (or something is not printed!!)
            
            //LOG(INFO) << "foo" << std::endl;
            
            if (wait())
            {
                addClient();
            }
            else
            {
                receive();
            }
        }
        m_listener.close();
        LOG(INFO) << "Server closed." << std::endl;
    }
        
    bool Server::wait()
    {
        return false;
    }
    
    void Server::receive()
    {
        //std::cout << "Do some stuff" << std::endl;
    }
        
    void Server::addClient()
    {
        std::cout << "Client added" << std::endl;
    }
    
    void Server::shutdown()
    {
        m_running = false;
    }
        
}
