#include "SocketStream.hpp"

#include <unistd.h>

namespace ryuuk
{
    ReceiveResult toResult(ssize_t size)
    {
        if (size == 0)
            return ReceiveResult::Disconnected;
        if (size < 0)
            return ReceiveResult::Error;
        return ReceiveResult::Success;
    }

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

    std::size_t SocketStream::send(std::string_view data)
    {
        std::size_t totalSent = 0;
        ssize_t sent = 0;

        while (totalSent < data.size())
        {
            if (0 > (sent = ::send(m_socketfd, (const void *)(data.data() + totalSent), data.size() - totalSent, 0)))
            {
                LOG(ERROR) << "send() : Error in sending data to remote client" << std::endl;
                return totalSent;
            }

            totalSent += sent;
        }

//         LOG(DEBUG) << "Sent data to remote client" << std::endl;

        return totalSent;
    }

    std::pair<ReceiveResult, std::string_view> SocketStream::receive()
    {
        ssize_t recvd = 0;

        if (0 > (recvd = recv(m_socketfd, m_rwbuffer,
                    DEFAULT_MSG_LENGTH, 0)))
        {
            LOG(ERROR) << "recv() : Error in receving data from remote client. errno: " << errno << std::endl;
        }
        auto result = toResult(recvd);
        auto result_view = result == ReceiveResult::Success
                            ? std::string_view{m_rwbuffer, static_cast<std::size_t>(recvd)}
                            : std::string_view{};

        return {result, result_view};
    }

}
