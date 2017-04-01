#ifndef SOCKETLISTENER_H
#define SOCKETLISTENER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

namespace ryuuk
{

    class SocketListener
    {
    public:
        SocketListener();
        ~SocketListener();
        listen(int port);
        StreamSocket accept();
    private:
        int m_socketfd;
    };

}
#endif // SOCKETLISTENER_H
