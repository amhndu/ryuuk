#ifndef SOCKETSELECTOR_HPP
#define SOCKETSELECTOR_HPP


#include "Socket.hpp"

#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


namespace ryuuk
{

    class SocketSelector
    {
    public:

        SocketSelector();

        ~SocketSelector();

        void clear();

        void add(Socket& socket);
        void remove(Socket& socket);

        bool wait(unsigned timeout = DEFAULT_TIMEOUT); // in microseconds

        bool isReady(Socket& socket) const;

    private:

        fd_set m_socketsFds;

        fd_set m_readyFds;

        int m_maxFd;

//        timeval m_tv;

        unsigned m_count;
    };

}

#endif // SOCKETSELECTOR_HPP
