#include "SocketStream.hpp"

#include <iostream>
#include <cstdlib>
#include <cstring>

namespace ryuuk
{
    SocketStream::SocketStream()
    {
        LOG(DEBUG) << "Empty SocketStream object created." << std::endl;
    }

    SocketStream::SocketStream(int sockfd, sockaddr_storage& sockinfo) : Socket(sockfd),
                                                                         m_clientAddr(sockinfo)
    {
        //m_clientInfo = new struct addrinfo;
        //memcpy(m_clientInfo, &sockinfo, sizeof m_clientInfo);

        LOG(DEBUG) << "SocketStream object created." << std::endl;
    }

    SocketStream::~SocketStream()
    {
        //freeaddrinfo(m_clientInfo);
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
            if (0 > (sent = ::send(m_socketfd, (const void *)(data + totalSent), size_t(len - totalSent), 0)))
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
        int recvd = 0;

        if (0 > (recvd = recv(m_socketfd, m_rwbuffer,
                    DEFAULT_MSG_LENGTH, 0)))
        {
            LOG(ERROR) << "recv() : Error in receving data from remote client" << std::endl;

        }
        else
            LOG(DEBUG) << "Recevied data from remote client" << std::endl;

        return recvd;
    }

}
