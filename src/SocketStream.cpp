#include "SocketStream.hpp"

#include <unistd.h>

namespace ryuuk
{
    SocketStream::SocketStream()
    {
    }

    SocketStream::SocketStream(int sockfd, sockaddr_storage& sockinfo) : Socket(sockfd),
                                                                         m_clientAddr(sockinfo)
    {
    }

    SocketStream::SocketStream(SocketStream&& other) noexcept   : Socket(other.m_socketfd),
                                                                  m_clientAddr(other.m_clientAddr)
    {
        other.m_socketfd = INVALID_SOCKET_FD;
    }


    SocketStream::~SocketStream()
    {
        if (valid())
        {
            shutdown();
            close(m_socketfd);
        }
    }

    void SocketStream::shutdown()
    {
        ::shutdown(m_socketfd, SHUT_RDWR);
    }

    int SocketStream::send(const char* data, size_t len)
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

//         LOG(DEBUG) << "Sent data to remote client" << std::endl;

        return totalSent;
    }

    std::pair<int, const char*> SocketStream::receive()
    {
        int recvd = 0;

        if (0 > (recvd = recv(m_socketfd, m_rwbuffer,
                    DEFAULT_MSG_LENGTH, 0)))
        {
            LOG(ERROR) << "recv() : Error in receving data from remote client. errno: " << errno << std::endl;
        }
//         else if (recvd > 0)
//             LOG(DEBUG) << "Recevied data from remote client" << std::endl;
//         else
//             LOG(DEBUG) << "Socket " << m_socketfd << " disconnected." << std::endl;

        return std::make_pair(recvd, m_rwbuffer);
    }

}
