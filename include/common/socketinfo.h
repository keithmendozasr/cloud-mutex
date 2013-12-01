#ifndef CLOUDMUTEX_COMMON_SOCKETINFO_H
#define CLOUDMUTEX_COMMON_SOCKETINFO_H

#include <stdexcept>
#include <string>
#include <memory>

#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <log4cplus/logger.h>

namespace cloudmutex
{

class SocketInfo
{
public:
    explicit SocketInfo();
    SocketInfo(const int &sockfd, const sockaddr_storage *addr);
    
    inline int getSocket() const
    {
        if(sockfd == -1)
            throw std::logic_error("Socket not created");

        return sockfd;
    }
    
    inline void closeSocket()
    {
        if(sockfd != -1)
        {
            shutdown(sockfd, SHUT_RDWR);
            close(sockfd);
            sockfd = -1;
        }
    }

    const std::string getSocketIP() const;
    
    inline const struct sockaddr_storage getAddrInfo() const
    {
        return addrInfo;
    }
    
    const size_t getAddrInfoSize() const;
    
protected:
    void initSocket(const unsigned int &port, const std::string &host = "");
    void setAddrInfo(const sockaddr_storage *addr);

private:
    log4cplus::Logger logger;
    struct sockaddr_storage addrInfo;
    int sockfd;
};

} //namespace cloudmutex
#endif
