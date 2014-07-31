#ifndef CLOUDMUTEX_COMMON_SOCKETINFO_H
#define CLOUDMUTEX_COMMON_SOCKETINFO_H

#include <stdexcept>
#include <system_error>
#include <string>
#include <memory>

#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <log4cplus/logger.h>

namespace cloudmutex
{

/**
 * Class to manage sockets
 */
class SocketInfo
{
public:
    /**
     * Default constructor
     */
    explicit SocketInfo();

    /**
     * Constructor taking socket descriptor and pointer to sockaddr_storage
     *
     * \param sockfd Socket descriptor
     * \param addr Pointer to sockaddr_storage structure associated with the sockfd
     */
    SocketInfo(const int &sockfd, const sockaddr_storage *addr, const size_t &addrSize);

    virtual ~SocketInfo()
    {
        closeSocket();
    };
    
    /**
     * Return the socket descriptor
     */
    inline int getSocket() const
    {
        if(sockfd == -1)
            throw std::logic_error("Socket not created");

        return sockfd;
    }
    
    /**
     * Close the socket connection
     */
    inline void closeSocket()
    {
        if(sockfd != -1)
        {
            shutdown(sockfd, SHUT_RDWR);
            close(sockfd);
            sockfd = -1;
        }
    }

    /**
     * Get the IP address the socket is connected to
     */
    const std::string getSocketIP() const;
    
    /**
     * Access the sockaddr_storage structure associated with the socket connection
     */
    inline const struct sockaddr_storage getAddrInfo() const
    {
        return addrInfo;
    }
    
    /**
     * Get the address info structure's size
     */
    const size_t getAddrInfoSize() const
    {
        return addrInfoSize;
    }
    
    /**
     * Attempt to read from client.
     *
     * \throws std::invalid_argument data parameter is null
     * \throws std::system_error Error encountered while waiting for message to arrive
     * \throws TimeoutException when timeout reached while waiting for data to arrive
     * \param data Buffer to place received data
     * \param dataSize Size of data
     * \return Number of bytes received
     */
    const size_t readData(char *data, const size_t &dataSize);

    /**
     * Attempt to send to client
     *
     * \throws std::system_error Error encountered while waiting for socket to be ready to send message
     * \throws TimeoutException when timeout reached while waiting for socket to be ready to send message
     * \param msg Data to send
     * \param msgSize Size of msg
     * \return Number of bytes sent
     */
    const size_t writeData(const char *msg, const size_t &msgSize);

protected:
    /**
     * Initialize a socket connection instance
     *
     * \throws invalid_argument if port param not valid port number
     * \throws logic_error is socket instance already created
     * \throws runtime_error if unable to create socket instance
     * \param port Port number to use with socket connection
     * \param host Host to connect to as client, or host to listen from as server
     */
    void initSocket(const unsigned int &port, const std::string &host = "");

    /**
     * Set the sockaddr_storage structure to associate to this class' instance
     *
     * \param addr Pointer to sockaddr_storage to asssociate
     */
    void setAddrInfo(const sockaddr_storage *addr, const size_t &addrSize);

    inline void throwSystemError(const int &err, const std::string &msg)
    {
        throw std::system_error(err, std::system_category(), msg);
    }

private:
    log4cplus::Logger logger;
    struct sockaddr_storage addrInfo;
    size_t addrInfoSize;
    int sockfd;

    fd_set readFd, writeFd;
    timeval timeout;
};

} //namespace cloudmutex
#endif
