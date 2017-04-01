#include "SocketListener.h"

#include <cstdlib>
#include <iostream>

namespace ryuuk
{
    SocketListener::SocketListener() :
        m_socketfd(0),
        m_serverInfo(nullptr)
    {

    }

    bool SocketListener::listen(int port, int n)
    {
        int status;
        addrinfo hints;

        std::memset(&hints, 0, sizeof hints); // make sure the struct is empty
        hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
        hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
        hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

        if (m_serverInfo)
        {
            freeaddrinfo(m_serverInfo);
        }

        if ((status = getaddrinfo(NULL, std::to_string(port), &hints, &m_serverInfo)) != 0)
        {
            std::cerr << "getaddrinfo error:" << gai_strerror(status) << std::endl;
            return false;
        }

        m_socketfd = socket(m_serverInfo->ai_family, m_serverInfo->ai_socktype, m_serverInfo->ai_protocol);
        if (m_socketfd < 0)
        {
            std::cerr << "Error in creating socket file descriptor.\n" << std::endl;
            return false;
        }

        if (bind(m_socketfd, m_serverInfo->ai_addr, m_serverInfo->ai_addrlen) < 0)
        {
            std::cerr << "Error in binding socket to port" << std::endl;
            return false;
        }

        ::listen(m_socketfd, n);

        return true;
    }

    StreamSocket SocketListener::accept()
    {
        int client_sockfd = ::accept(m_socketfd, foo, bar);
        return StreamSocket{client_sockfd};
    }

    SocketListener::~SocketListener()
    {
        freeaddrinfo(m_serverInfo);
    }


}