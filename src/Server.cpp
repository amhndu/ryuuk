#include "Server.hpp"

#include <fstream>
#include <algorithm>

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

    Server::Server() : m_listener() ,
                       m_clients()  ,
                       m_running(true)
    {
        LOG(INFO) << "Server object created." << std::endl;

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
            LOG(ERROR) << "[FATAL] Ryuuk config file error: Unable to read config file! Terminating..." << std::endl;
            throw std::runtime_error("[FATAL] \'" + SERVER_CONFIG_FILE + "\': No such file exists or it is corrupted!");
        }

        // Read config options...
        std::string line;
        const std::string fields[] = {"IP", "Port", "Connections"};
        enum { Connection, None } section = None;
        unsigned int line_no = 0;
        while (std::getline(configFile, line))
        {
            line = line.substr(0, line.find('#')); // Truncate string until before a comment starts
            line = rtrim(ltrim(line));

            if (/*line[0] == '#' || */line.empty())
                continue;
            else if (line == "[Connection]")
            {
                section = Connection;
            }
            else if (section == Connection || section == None) // Being lenient, whatevs be the section
            {
                auto divider = line.find("=");
                std::string field  = ltrim(rtrim(line.substr(0, divider)));
                std::string value = ltrim(rtrim(line.substr(divider + 1)));
                try
                {
                    if (field == "IP")
                        server_manifest.ip = value;
                    else if (field == "Port")
                        server_manifest.port = std::stoi(value);
                    else if (field == "Backlog")
                        server_manifest.backlog = std::stoi(value);
                    else
                    {
                        LOG(INFO) << "Invalid key in configuration file at Line " << line_no << std::endl;
                        continue;
                    }

                    LOG(INFO) << "Configured " << field << " to " << value << std::endl;
                }
                catch (const std::invalid_argument& e)
                {
                    LOG(INFO) << "Invalid value in configuration file at line " <<  line_no << std::endl;
                }
            }
            else
                LOG(ERROR) << "Invalid line in key configuration at Line " << line_no << std::endl;

            ++line_no;
        }

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
