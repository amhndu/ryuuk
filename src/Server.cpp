#include "Utility.hpp"
#include "Server.hpp"
#include "HTTP.hpp"

#include <fstream>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <functional>

namespace
{
    std::string emptyIfNull(const char* c_str)
    {
        if (c_str) return c_str;
        return {};
    }
}

namespace ryuuk
{


    Server::Server() : m_running(true)
    {
        LOG(INFO) << "Server object created." << std::endl;
    }

    void Server::init()
    {
        LOG(INFO) << "Parsing server configuration file..." << std::endl;
        parseConfigFile();

        LOG(INFO) << "Adding supported headers..." << std::endl;

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
    }

    Server::~Server()
    {
        LOG(INFO) << "Server object destroyed." << std::endl;
    }

    void Server::parseConfigFile()
    {
        if (m_configPath.empty())
        {
            std::string confDir = emptyIfNull(std::getenv("XDG_CONFIG_HOME"));
            if (confDir.empty())
                confDir = emptyIfNull(std::getenv("HOME")) + "/.config/";

            m_configPath = confDir + "ryuuk/" + SERVER_CONFIG_FILE;
        }

        std::ifstream configFile(m_configPath, std::ios::in);

        if (!configFile.good() || !configFile.is_open())
        {
            LOG(ERROR) << "[FATAL] Ryuuk config file error: Unable to read config file! Terminating..." << std::endl;
            throw std::runtime_error("[FATAL] \'" + m_configPath + "\': No such file exists or it is corrupted!");
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

                Response::mimeTypes[field] = value;

                LOG(DEBUG) << "Added MIME type: " + field + ": " + value << std::endl;
            }
            else
                LOG(ERROR) << "Invalid line in key configuration at Line " << line_no << std::endl;

            ++line_no;
        }

        LOG(INFO) << "Parsed and applied server configuration from \'" + m_configPath << "\'." << std::endl;
    }

    void Server::run()
    {
        LOG(INFO) << "Server running." << std::endl;
        while(m_running)
        {
            SocketStream socket = m_listener.accept();    // Blocks until a new connection
            if (socket.valid())
            {
                LOG(DEBUG) << "Accepting new connection" << std::endl;

                std::lock_guard<std::mutex> guard(m_queueMutex);
                if (!m_cleanupQueue.empty())
                {
                    LOG(DEBUG) << "Doing worker thread cleanup" << std::endl;
                }
                for (int fd : m_cleanupQueue)
                {
                    auto i = m_connections.find(fd);
                    if (i == m_connections.end())
                        throw std::runtime_error("fd " + std::to_string(fd) + " from m_cleanupQueue not found in map");
                    LOG(DEBUG) << "Removing worker/socket " << fd << std::endl;
                    i->second.join();
                    m_connections.erase(i);
                }
                m_cleanupQueue.clear();

                // Insertion must be after clean-up, otherwise, we'll delete a thread before it's socket was closed
                int fd = socket.getSocketFd();  // Copy the fd before we move the socket object (and thus invalidate it)
                m_connections.emplace(fd, std::thread(&Server::worker, this, std::move(socket)));
            }
            else
            {
                LOG(ERROR) << "accept() error: Unable to establish connection with remote socket. errno: " << errno << std::endl;
            }
        }

        LOG(DEBUG) << "Shutting down sockets for remaining worker threads and waiting for them to finish" << std::endl;
        for (auto i = m_connections.begin(); i != m_connections.end(); ++i)
        {
            // Shut down the socket, this will cause the recv in the thread to fail and thus exit
            // This is *probably* not a very good design..
            ::shutdown(i->first, SHUT_RDWR);
            // Wait for it to finish
            i->second.join();
        }

        LOG(INFO) << "Server closed." << std::endl;
    }

    void Server::worker(SocketStream &&sock)
    {
        int received = -1;
        SocketStream socket(std::move(sock));
        HTTP::Manifest manifest;
        std::string request;
        bool leftOverAttempt = false;
        LOG(DEBUG) << "Worker starting up with socket " << socket.getSocketFd() << std::endl;
        do
        {
            if (!leftOverAttempt)
            {
                const char* buffer = nullptr;
                std::tie(received, buffer) = socket.receive();
                request.append(buffer, buffer + received);
            }
            else
                leftOverAttempt = false;

            if (request.size() > 4096)   // An arbitrary ceiling
            {
                // Forget replying, they're sending gibberish anyway.
                break;
            }

            if (received == 0)
            {
                LOG(DEBUG) << "Removing socket " << socket.getSocketFd() << std::endl;
            }
            else if (received < 0)
            {
                LOG(ERROR) << "Receive error with socket " << socket.getSocketFd() << " and errno " << errno << std::endl;
            }
            else
            {
                LOG(DEBUG) << "Received data from " << socket.getSocketFd() << std::endl;
                HTTP http;
                std::string response(http.buildResponse(request, manifest));

                // If bytesRead is 0, that means the request is incomplete (or possibly malformed)
                // We thus try and wait for it to complete in the next attempt.
                if (manifest.bytesRead != 0)
                {
                    /*
                    LOG(DEBUG) << "Request dump:\n" << conv(request) << std::endl;
                    LOG(DEBUG) << "Sending response to client " << socket.getSocketFd() << ". Dump:\n" <<
                                conv(res) << std::endl;
                    */

                    if (socket.send(response.c_str(), response.size()) != response.size())
                    {
                        LOG(INFO) << "Couldn't send HTTP response. errno: " << errno << std::endl;
                        // FIXME TODO Should we just remove the socket or maybe try doing this a couple of more times ?
                        // Also BIG files.
                        break;
                    }

                    // Bytes that haven't been consumed will be used for the next request
                    request.erase(0, manifest.bytesRead);
                    if (!request.empty())
                        leftOverAttempt = true;
                    /*LOG(DEBUG) << "Request left-over dump:\n" << conv(request) << std::endl;*/
                }
            }
        }
        while (manifest.keepAlive && received > 0);

        std::lock_guard<std::mutex> guard(m_queueMutex);
        m_cleanupQueue.push_back(socket.getSocketFd());
        LOG(DEBUG) << "Adding socket " << socket.getSocketFd() << " to cleanup queue. "
                   << "Worker thread closing" << std::endl;
    }

    void Server::shutdown()
    {
        m_running = false;
    }

    void Server::setConfigFile(const std::string& file)
    {
        m_configPath = file;
        LOG(INFO) << "Ryuuk configuration file set to \'" + file + "\'" << std::endl;
    }

}
