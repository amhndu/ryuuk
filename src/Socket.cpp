#include "Socket.hpp"

namespace ryuuk
{
    Socket::Socket() : m_socketfd(INVALID_SOCKET_FD)
    {
    }

    Socket::Socket(int socketfd)
    {
        m_socketfd = socketfd;
    }

//    void Socket::setSocketFd(const int socketfd)
//    {
//        m_socketFd = socketfd;
//    }

}
