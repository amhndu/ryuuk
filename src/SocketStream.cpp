#include "SocketStream.hpp"

namespace ryuuk
{
    SocketStream::SocketStream() : m_clientInfo(nullptr),
                                   m_socketfd(-1)
    {
        std::cerr << "Empty SocketStream object created." << std::endl;
    }
    
    SocketStream::SocketStream(int sockfd)
    {
        m_socketfd = sockfd;
        std::cerr << "SocketStream object created." << std::endl;
    }

    ~SocketStream::SocketStream()
    {
        freeaddrinfo(m_clientInfo);
        std::cerr << "Destroyed SocketStream object" << std::endl;
    }
    
    bool SocketStream::create()
    {
    }
    
    bool SocketStream::drop()
    {
    }

    int SocketStream::send(SocketStream& remote)
    {
        int totalSent = -1, sent = -1;
        
        while (totalSent + 1 < DEFAULT_MSG_LENGTH)
        {
            if (0 > (sent = send(remote.getSocketFd(), m_rwbuffer, strlen(m_rwbuffer), 0)))
            {
                std::cerr << "send() : Error in sending data to remote client" << std::endl;
                return totalSent + 1;
            }
            
            totalSent += sent;
        }
        
        std::cerr << "Sent data to remote client" << std::endl;
        
        return totalSent + 1;
    }

    bool SocketStream::receive(SocketStream& remote)
    {
        int totalRecvd = -1, recvd = -1;
        
        while (totalRecvd + 1 < DEFAULT_MSG_LENGTH)
        {
            if (0 > (recvd = recv(remote.getSocketFd(), m_rwbuffer, strlen(m_rwbuffer), 0)))
            {
                std::cerr << "recv() : Error in receving data from remote client" << std::endl;
                return totalRecvd + 1;
            }
            
            totalRecvd += recvd;
        }
        
        return totalRecvd + 1;
    }
    
    int SocketStream::getSocketFd()
    {
        return m_socketfd;
    }
}
