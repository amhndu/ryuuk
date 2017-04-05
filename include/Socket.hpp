#ifndef SOCKET_HPP_INCLUDED
#define SOCKET_HPP_INCLUDED


#include "Log.hpp"


namespace ryuuk
{

    const int INVALID_SOCKET_FD = -1;

    class Socket
    {
    public:

        Socket();

        Socket(int socketfd);

        //void setSocketFd(const int socketfd);

        inline int getSocketFd() const
        {
            return m_socketfd;
        }

    protected:

        int m_socketfd;
    };

}

#endif // SOCKET_HPP_INCLUDED
