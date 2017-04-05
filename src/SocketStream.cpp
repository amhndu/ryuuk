#include "SocketStream.hpp"

#include <iostream>
#include <cstdlib>
#include <cstring>

namespace ryuuk
{
    SocketStream::SocketStream() : m_clientInfo(nullptr)
    {
        LOG(DEBUG) << "Empty SocketStream object created." << std::endl;
    }

    SocketStream::SocketStream(int sockfd, struct addrinfo sockinfo) : Socket(sockfd)
    {
        m_clientInfo = new struct addrinfo;
        memcpy(m_clientInfo, &sockinfo, sizeof m_clientInfo);

        LOG(DEBUG) << "SocketStream object created." << std::endl;
    }

    SocketStream::~SocketStream()
    {
        freeaddrinfo(m_clientInfo);
        LOG(DEBUG) << "Destroyed SocketStream object" << std::endl;
    }

    bool SocketStream::create()
    {
    }

    bool SocketStream::drop()
    {
    }

    size_t SocketStream::send(const char* data, size_t len)
    {
        int totalSent = 0, sent = 0;

        while (totalSent < len)
        {
            if (0 > (sent = ::send(getSocketFd(), (const void *)(data + totalSent), size_t(len - totalSent), 0)))
            {
                LOG(ERROR) << "send() : Error in sending data to remote client" << std::endl;
                return totalSent;
            }

            totalSent += sent;
        }

        LOG(DEBUG) << "Sent data to remote client" << std::endl;

        return totalSent;
    }

    int SocketStream::receive()
    {
        int totalRecvd = 0, recvd = 0;

        while (totalRecvd < DEFAULT_MSG_LENGTH)
        {
            if (0 > (recvd = recv(getSocketFd(), m_rwbuffer + totalRecvd,
                        DEFAULT_MSG_LENGTH - totalRecvd, 0)))
            {
                LOG(ERROR) << "recv() : Error in receving data from remote client" << std::endl;
                return totalRecvd;
            }

            totalRecvd += recvd;
        }

        LOG(DEBUG) << "Recevied data from remote client" << std::endl;

        return totalRecvd;
    }

    bool SocketStream::valid()
    {
        return getSocketFd() > 0;
    }

}
