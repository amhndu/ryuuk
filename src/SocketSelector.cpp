#include "Log.hpp"
#include "SocketSelector.hpp"

namespace ryuuk
{

    SocketSelector::SocketSelector() : m_maxFd(INVALID_SOCKET_FD) ,
                                       m_count(0)
    {
        FD_ZERO(&m_socketsFds);
        FD_ZERO(&m_readyFds);
    }

    void SocketSelector::clear()
    {
        throw std::runtime_error("SocketSelector::clear() not implemented.");
    }

    void SocketSelector::add(Socket& socket)
    {
        int socketfd = socket.getSocketFd();

        if (!socket.valid())
        {
            LOG(ERROR) << "Invalid `Socket` object passed to selector, skipping." << std::endl;
            return;
        }

        if (m_count >= FD_SETSIZE)
        {
            LOG(ERROR) << "[FATAL] A new socket cannot be added to the selector because FD_SETSIZE's limit for your OS was exceeded." << std::endl;
            // throw ...
        }

        if (FD_ISSET(socketfd, &m_socketsFds))
        {
            LOG(INFO) << "Socket with FD: " << std::to_string(socketfd) << " already added to selector, skipping." << std::endl;
            return;
        }

        // New socket

        if (socketfd >= FD_SETSIZE)
        {
            LOG(ERROR) << "[FATAL] ryuuk::SocketSelector::add(): Socket descriptor is too high!" << std::endl;
            return;
        }

        m_count++;
        m_maxFd = std::max(m_maxFd, socketfd);
        FD_SET(socketfd, &m_socketsFds);
        LOG(DEBUG) << "Added socket with fd " << socketfd << " to the set" << std::endl;
    }

    void SocketSelector::remove(Socket& socket)
    {
        if (!socket.valid())
        {
            LOG(ERROR) << "Attempting to remove invalid socket from selector" << std::endl;
            return;
        }

        int socketfd = socket.getSocketFd();
        if (socketfd >= FD_SETSIZE)
        {
            LOG(ERROR) << "[FATAL] ryuuk::SocketSelector::remove(): Socket descriptor is too high!" << std::endl;
            return;
        }

        if (!FD_ISSET(socketfd, &m_socketsFds))
        {
            LOG(INFO) << "Socket with FD: " << std::to_string(socketfd) << " was never present in selector to remove." << std::endl;
            return;
        }

        --m_count;
        // no can do with maxfd
        FD_CLR(socketfd, &m_socketsFds);
        FD_CLR(socketfd, &m_readyFds);
    }

    bool SocketSelector::wait(unsigned timeout)
    {
        timeval tOut;

        // TODO Read from config ?
        tOut.tv_sec = timeout / 1000000;
        tOut.tv_usec = timeout % 1000000;

        m_readyFds = m_socketsFds;

        int count = select(m_maxFd + 1, &m_readyFds, nullptr, nullptr, &tOut);

        return count > 0;
    }

    bool SocketSelector::isReady(Socket& socket) const
    {
        int socketfd = socket.getSocketFd();

        if (!socket.valid())
        {
            LOG(ERROR) << "Invalid `Socket` object passed to selector, skipping." << std::endl;
            return false;
        }

        if (socketfd >= FD_SETSIZE)
        {
            LOG(ERROR) << "[FATAL] ryuuk::SocketSelector::isReady(): Socket descriptor is too high!" << std::endl;
            return false;
        }

        return FD_ISSET(socketfd, &m_readyFds) != 0;

    }

}
