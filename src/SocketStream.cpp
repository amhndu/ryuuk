#include "SocketStream.hpp"

#include <iostream>
#include <cstdlib>

namespace ryuuk
{
    SocketStream::SocketStream() : m_clientInfo(nullptr),
                                   m_socketfd(-1)
    {
        std::cerr << "Empty SocketStream object created." << std::endl;
    }

    SocketStream::SocketStream(int sockfd)
    {
        m_socketfd = sockfd;
        std::cerr << "SocketStream object created." << std::endl;
    }

    SocketStream::~SocketStream()
    {
        freeaddrinfo(m_clientInfo);
        std::cerr << "Destroyed SocketStream object" << std::endl;
    }

    bool SocketStream::create()
    {
    }

    bool SocketStream::drop()
    {
    }

    std::size_t SocketStream::send(const char* data, std::size_t len)
    {
        int totalSent = 0, sent = 0;

        while (totalSent < len)
        {
            if (0 > (sent = send(m_socketfd, data + totalSent, len - totalSent, 0)))
            {
                std::cerr << "send() : Error in sending data to remote client" << std::endl;
                return totalSent;
            }

            totalSent += sent;
        }

        std::cout << "Sent data to remote client" << std::endl;

        return totalSent;
    }

     SocketStream::receive()
    {
        int totalRecvd = 0, recvd = 0;

        while (totalRecvd < DEFAULT_MSG_LENGTH)
        {
            if (0 > (recvd = recv(m_socketfd, m_rwbuffer + totalRecvd,
                        DEFAULT_MSG_LENGTH - totalRecvd, 0)))
            {
                std::cerr << "recv() : Error in receving data from remote client" << std::endl;
                return totalRecvd;
            }

            totalRecvd += recvd;
        }

        return totalRecvd;
    }

    int SocketStream::getSocketFd()
    {
        return m_socketfd;
    }
}
