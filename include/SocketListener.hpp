/**
*  Ryuuk - Simple, multi-threaded, C++ webserver
* -----------------------------------------------
* 
*  SockerListener
* ----------------
*  TCP socket which listens for incoming client
*  requests.
*/


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
    
        /**
        * Default constructor. Born this way.
        * 
        * Creates an invalid socket listener object.
        */
        SocketListener();
        
        /**
        * Destructor. Stairway to heaven.
        */
        ~SocketListener();
        
        /**
        * Bind a socket on the specified port and `listen`
        * for incoming client connections, queuing atmost
        * `backlog` no. of client requests.
        *
        * @param port - The port to listen on
        * @param backlog - Max. no. of requests to queue
        *
        * @return true if a socket was bound to `port`
        */
        bool listen(int port, int backlog);
        
        /**
        * Accept a client connection.
        *
        * @return A `SocketStream` object with relevant
                  remote client info which will be processed
                  later for HTTP requests
        */
        SocketStream accept();
        
        /**
        * Destroy the listener
        */
        void close();
        
    private:
    
        /* The socket file descriptor of the TCP
           socket of the server. */
        int m_socketfd;
    };

}

#endif // SOCKETLISTENER_HPP
