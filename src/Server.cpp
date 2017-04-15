#include "Server.hpp"
#include "HTTP.hpp"

#include <fstream>
#include <algorithm>
#include <sstream>
#include <iomanip>

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
                       m_running(true),
                       m_configFile{SERVER_CONFIG_FILE}
    {
        LOG(INFO) << "Server object created." << std::endl;
    }

    void Server::init()
    {
        LOG(INFO) << "Parsing server configuration file..." << std::endl;
        parseConfigFile();

        LOG(INFO) << "Attempting to bind listener (SocketListener object)..." << std::endl;
        if (m_listener.listen(server_manifest.port, server_manifest.backlog))
            LOG(INFO) << "Successfully bound listener on port \'" + std::to_string(server_manifest.port) + "\'." << std::endl;
        else
        {
            LOG(ERROR) << "[FATAL] Server could not bind listener on port \'"
                       << std::to_string(server_manifest.port) << "\'. Exiting..." << std::endl;
            throw std::runtime_error("Server could not bind listener on port");
        }

        m_running = true;
        m_selector.add(m_listener);

        LOG(INFO) << "Listening for incoming connections..." << std::endl;
    }

    Server::~Server()
    {
        //m_listener.~SocketListener(); : Undefined behaviour! ouch!
        LOG(INFO) << "Server object destroyed." << std::endl;
    }

    std::string conv(const std::string s)
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

    void Server::parseConfigFile()
    {
        std::ifstream configFile(m_configFile, std::ios::in);

        if (!configFile.good() || !configFile.is_open())
        {
            LOG(ERROR) << "[FATAL] Ryuuk config file error: Unable to read config file! Terminating..." << std::endl;
            throw std::runtime_error("[FATAL] \'" + m_configFile + "\': No such file exists or it is corrupted!");
        }

        // Read config options...
        std::string line;
        const std::string fields[] = {"IP", "Port", "Connections"};
        enum { Connection, MIME, None } section = None;
        unsigned int line_no = 0;
        while (std::getline(configFile, line))
        {
            line = line.substr(0, line.find('#')); // Truncate string until before a comment starts
            line = rtrim(ltrim(line));

            if (/*line[0] == '#' || */line.empty())
                continue;
            else if (line == "[Connection]")
            {
                LOG(DEBUG) << "Parsing connection configuration options..." << std::endl;
                section = Connection;
            }
            else if (line == "[MIME]")
            {
                LOG(DEBUG) << "Parsing MIME configuration options..." << std::endl;
                section = MIME;
            }
            //else if (section == Connection || section == None) // Being lenient, whatevs be the section
            else if (section == Connection)
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
            else if (section == MIME)
            {
                auto divider = line.find("=");
                std::string field  = ltrim(rtrim(line.substr(0, divider)));
                std::string value = ltrim(rtrim(line.substr(divider + 1)));

                server_manifest.mime_types[field] = value;

                LOG(DEBUG) << "Added MIME type: " + field + ": " + value << std::endl;
            }
            else
                LOG(ERROR) << "Invalid line in key configuration at Line " << line_no << std::endl;

            ++line_no;
        }

        LOG(INFO) << "Parsed and applied server configuration from \'" + m_configFile << "\'." << std::endl;

        HTTP::mimeTypes = server_manifest.mime_types;
    }

    void Server::run()
    {
        LOG(INFO) << "Server running." << std::endl;
        while(m_running)
        {
            // Wait until some socket is active
            if (m_selector.wait())
            {
                // Try doing either
                addClient();
                receive();
            }
        }
        LOG(INFO) << "Server closed." << std::endl;
    }

    void Server::receive()
    {
        for (auto sock_i = m_clients.begin(); sock_i != m_clients.end(); )
        {
            if (m_selector.isReady(*sock_i))
            {
                int recieved = -1;
                const char* buffer = nullptr;
                std::tie(recieved, buffer) = sock_i->receive();

                if (recieved == 0)
                {
                    LOG(DEBUG) << "Removing socket " << sock_i->getSocketFd() << std::endl;
                    m_selector.remove(*sock_i);
                    sock_i = m_clients.erase(sock_i);
                    continue;
                }
                else if (recieved < 0)
                {
                    LOG(ERROR) << "Receive error with socket " << sock_i->getSocketFd() << " and errno " << errno << std::endl;
                }
                else
                {
                    LOG(DEBUG) << "Received request from " << sock_i->getSocketFd() << /*". Dump:\n" <<
                                conv({buffer, buffer+recieved}) <<*/ std::endl;
                }

                HTTP http;
                std::string res(http.buildResponse({buffer, buffer + recieved}));
//                 LOG(DEBUG) << "Sending response to client " << sock_i->getSocketFd() << ". Dump:\n" <<
//                                 conv(res) << std::endl;

                if (sock_i->send(res.c_str(), res.size()) != res.size())
                {
                    LOG(INFO) << "Couldn't send HTTP response. errno: " << errno << std::endl;
                    // FIXME TODO Should we just remove the socket or maybe try doing this a couple of more times ?
                    sock_i = m_clients.erase(sock_i);
                    continue;
                }

            }

            ++sock_i;
        }
    }

    void Server::addClient()
    {
        if (m_selector.isReady(m_listener))
        {
            LOG(DEBUG) << "Accepting new connection" << std::endl;
            auto &&socket = m_listener.accept();
            if (socket.valid())
            {
                m_clients.push_back(std::move(socket));
                m_selector.add(m_clients.back());
            }
            else
            {
                LOG(ERROR) << "accept() error: Unable to establish connection with remote socket" << std::endl;
            }
        }
    }

    void Server::shutdown()
    {
        m_running = false;
    }

    void Server::setConfigFile(const std::string& file)
    {
        m_configFile = file;
        LOG(INFO) << "Ryuuk configuration file set to \'" + file + "\'" << std::endl;
    }

}
