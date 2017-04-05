/*
 *  Ryuuk
 * -------
 *  Ryuuk is an upcoming webserver written by Shinigamis
 *
 * SocketStream - A high level abstraction of a TCP socket
 *
 * TODO: Everything
 *
 */

#ifndef SOCKETSTREAM_HPP
#define SOCKETSTREAM_HPP


#include "Socket.hpp"

#include <cstddef>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>


namespace ryuuk
{

    class SocketStream : public Socket
    {
    public:

        /* Default buffer length for send/receive operations */
        static constexpr int DEFAULT_MSG_LENGTH = 4096; // bytes

        /**
        * Default constructor. Creates an empty/invalid TCP
        * socket object with an invalid socket.
        */
        SocketStream();

        /**
        * Create a TCP socket object with a valid
        * network socket.
        *
        * @param sockfd - A valid socket descriptor ID
        */
        SocketStream(int sockfd, struct addrinfo sockinfo);

        /**
        * Destructor. Leave out all the rest.
        */
        ~SocketStream();

        /**
        * Create a TCP socket
        *
        * @return true if a socket was successfully created
        */
        bool create();

        /**
        * Destroy a TCP socket
        *
        * @return true if a socket was successfully let go
        */
        bool drop();

        /**
        * High level function to send data to
        * a remote TCP socket.
        *
        * @param data - pointer to data
        * @param len - size of data (in bytes)
        *
        * @return The no. of bytes sent
        *
        * NOTE: This funciton blocks the current
        * thread until all the data has been sent.
        */
        std::size_t send(const char* data, std::size_t len);
        //int send(SocketStream& remote);

        /**
        * High level method to receive data from a
        * remote TCP socket.
        *
        * @param remote - A reference to the remote
        *                 TCP socket object
        *
        * @return the no. of bytes received
        *
        * Note: This method blocks the current thread
        * until all the data has been received.
        */
        int receive();

        /**
        * Get the socket file descriptor
        *
        * @return The FD associated with this TCP socket object
        */
        inline int getSocketFd();

        /**
        * Helper to check if this SockStream object is
        * associated to a valid TCP socket or not.
        *
        * @return true if a valid TCP socket is associated
        */
        inline bool valid();

    private:

        /* Socket connection info */
        struct addrinfo *m_clientInfo;

        /* Temporary R/W buffer */
        char m_rwbuffer[DEFAULT_MSG_LENGTH];
    };
}

#endif // SOCKETSTREAM_HPP

