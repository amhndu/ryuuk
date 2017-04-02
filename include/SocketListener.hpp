#ifndef SOCKETLISTENER_HPP
#define SOCKETLISTENER_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "SocketStream.hpp"

namespace ryuuk
{

    class SocketListener
    {
    public:
        SocketListener();
        ~SocketListener();
        bool listen(int port, int backlog);
        SocketStream accept();
        
    private:
        int m_socketfd;
    };

}
#endif // SOCKETLISTENER_HPP
