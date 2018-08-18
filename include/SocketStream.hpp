/*
 *  Ryuuk
 * -------
 *  Ryuuk is an upcoming webserver written by Shinigamis
 *
 * SocketStream - A high level abstraction of a TCP socket
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
#include <string_view>


namespace ryuuk
{
    enum class ReceiveResult
    {
        Success,
        Disconnected,
        Error
    };

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
         * Delete copy assignment and copy constructor
         */
        SocketStream(const SocketStream& other) = delete;
        SocketStream& operator=(const SocketStream& other) = delete;

        /*
         * Move constructor
         */
        SocketStream(SocketStream&& other) noexcept;

        /**
        * Create a TCP socket object with a valid
        * network socket.
        *
        * @param sockfd - A valid socket descriptor ID
        */
        SocketStream(int sockfd, sockaddr_storage& sockinfo);

        /**
        * Destructor. Leave out all the rest.
        */
        ~SocketStream();

        /**
         * Shutdown the socket, with default flags
         */
        void shutdown();

        /**
        * High level function to send data to
        * a remote TCP socket.
        *
        * @param data - data to send
        *
        * @return The no. of bytes sent
        *
        * NOTE: This funciton blocks the current
        * thread until all the data has been sent.
        */
        std::size_t send(std::string_view data);

        /**
        * High level method to receive data from a
        * remote TCP socket.
        *
        * @return a read-only view of the received message
        *
        * Note: This method blocks the current thread
        * until all the data has been received.
        */
        std::pair<ReceiveResult, std::string_view> receive();

    private:

        /* Socket connection info */
        sockaddr_storage m_clientAddr;

        /* Temporary R/W buffer */
        char m_rwbuffer[DEFAULT_MSG_LENGTH];
    };
}

#endif // SOCKETSTREAM_HPP

