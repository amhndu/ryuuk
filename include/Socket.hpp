#ifndef SOCKET_HPP_INCLUDED
#define SOCKET_HPP_INCLUDED


#include "Log.hpp"


namespace ryuuk
{

    const int INVALID_SOCKET_FD = -1;
    const unsigned DEFAULT_TIMEOUT = 5; // sec

    class Socket
    {
    public:

        Socket() : m_socketfd(INVALID_SOCKET_FD)
        {
        }

        Socket(int socketfd)
        {
            m_socketfd = socketfd;
        }

        inline int getSocketFd() const
        {
            return m_socketfd;
        }

        /**
        * Helper to check if this SockStream object is
        * associated to a valid TCP socket or not.
        *
        * @return true if a valid TCP socket is associated
        */
        inline bool valid()
        {
            return m_socketfd > 0;
        }

    protected:

        int m_socketfd;
    };

}

#endif // SOCKET_HPP_INCLUDED
