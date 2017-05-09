/**
*  Ryuuk - Simple, multi-threaded, C++ webserver
* -----------------------------------------------
*
*  SockerListener
* ----------------
*  TCP socket which listens for incoming client
*  requests.
*/


#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "Log.hpp"
#include "SocketListener.hpp"


namespace ryuuk
{
    SocketListener::SocketListener() :
        Socket()
    {
        LOG(DEBUG) << "Created empty SocketListener" << std::endl;
    }

    bool SocketListener::listen(int port, int backlog)
    {
        int status;
        addrinfo hints;
        addrinfo *serverInfo = nullptr;

        std::memset(&hints, 0, sizeof hints); // make sure the struct is empty
        hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
        hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
        hints.ai_flags = AI_PASSIVE;     // fill in my IP for me


        if ((status = getaddrinfo(NULL, std::to_string(port).c_str(), &hints, &serverInfo)) != 0)
        {
            LOG(ERROR) << "getaddrinfo() error: " << gai_strerror(status) << std::endl;
            return false;
        }

        auto p = serverInfo;
        for (; p != nullptr; p = p->ai_next)
        {
            m_socketfd = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

            if (m_socketfd < 0)
            {
                LOG(ERROR) << "socket() error: Unable to create a socket, trying next result" << std::endl;
                continue;
            }

            int yes = 1;
            if (setsockopt(m_socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
            {
                LOG(ERROR) << "setsockopt() error: Unable to set socket options, trying next result" << std::endl;
                //return false;
                continue;
            }

            if (bind(m_socketfd, serverInfo->ai_addr, serverInfo->ai_addrlen) < 0)
            {
                LOG(ERROR) << "bind() error: Unable to bind socket to port \'" + std::to_string(port) + "\', trying next result" << std::endl;
                continue;
            }

            LOG(DEBUG) << "Successfully bound socket for listening" << std::endl;
            break;
        }

        freeaddrinfo(serverInfo);

        if (p == nullptr)
        {
            LOG(ERROR) << "Could not find an appropriate server info" << std::endl;
            return false;
        }

        if (::listen(m_socketfd, backlog) < 0)
        {
            LOG(ERROR) << "Error in listening" << std::endl;
            return false;
        }

        LOG(INFO) << "Now Listening for incoming requests on port \'" + std::to_string(port) + "\'" << std::endl;
        return true;
    }

    SocketStream SocketListener::accept()
    {
        sockaddr_storage client_info;
        socklen_t addr_size = sizeof(client_info);

        memset(&client_info, 0, sizeof client_info);

        int client_sockfd = ::accept(m_socketfd, reinterpret_cast<sockaddr*>(&client_info), &addr_size);

        if (0 > client_sockfd)
        {
            return SocketStream{};
        }

        return SocketStream{client_sockfd, client_info};
    }

    void SocketListener::close()
    {
        if (m_socketfd > 0)
        {
            ::close(m_socketfd);
            m_socketfd = -1;
            LOG(INFO) << "Closed listener socket" << std::endl;
        }
    }

    SocketListener::~SocketListener()
    {
        close();
    }


}
