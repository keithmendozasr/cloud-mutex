#ifndef CLOUDMUTEX_COMMON_SOCKETINFO_H
#define CLOUDMUTEX_COMMON_SOCKETINFO_H

#include <stdexcept>
#include <system_error>
#include <string>
#include <memory>

#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <log4cplus/logger.h>

#ifdef GTEST_ENABLED
#include "gtest/gtest_prod.h"
#endif

namespace cloudmutex
{

/**
 * Class to manage sockets
 */
class SocketInfo
{
    #ifdef GTEST_ENABLED
    friend class SocketInfoTest;

    FRIEND_TEST(SocketInfoTestgetSocket, SocketReady);

    FRIEND_TEST(SocketInfoTestgetSocketIP, V4);
    FRIEND_TEST(SocketInfoTestgetSocketIP, V6);

    FRIEND_TEST(SocketInfoTestwaitForReading, Timeout);
    FRIEND_TEST(SocketInfoTestwaitForReading, Fail);
    FRIEND_TEST(SocketInfoTestwaitForReading, Ready);

    FRIEND_TEST(SocketInfoTestreadData, Failed);
    FRIEND_TEST(SocketInfoTestreadData, Timeout);
    FRIEND_TEST(SocketInfoTestreadData, Disconnect);
    FRIEND_TEST(SocketInfoTestreadData, GoodRead);
    #endif

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

    virtual ~SocketInfo();
    
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
     * Wait for data to be available for reading
     *
     * \throws std::system_error Error encountered while waiting for message to arrive
     * \throws TimeoutException when timeout reached while waiting for data to arrive
     * \param timeout Number of seconds to wait for data. 0 for no timeout
     */
    void waitForReading(const unsigned int timeout = 0);

    /**
     * Attempt to read from client.
     *
     * \throws std::invalid_argument data parameter is null
     * \param data Buffer to place received data
     * \param dataSize Size of data
     * \return Number of bytes received
     */
    const size_t readData(char *data, const size_t &dataSize);

    /**
     * Wait for socket to be ready for writing
     *
     * \throws std::system_error Error encountered while waiting for socket to be ready to send message
     * \throws TimeoutException when timeout reached while waiting for socket to be ready to send message
     * \param timeout Number of seconds to wait for socket to be writable. 0 for no timeout
     */
    void waitForWriting(const unsigned int timeout = 0);

    /**
     * Attempt to send to client
     *
     * \param msg Data to send
     * \param msgSize Size of msg
     * \return Number of bytes sent
     */
    const size_t writeData(const char *msg, const size_t &msgSize);

protected:
    /**
     * Initialize a socket connection instance resolving the port and host
     *
     * \throws invalid_argument if port param not valid port number
     * \throws logic_error is socket instance already created
     * \throws runtime_error if unable to create socket instance
     * \param port Port number to use with socket connection
     * \param host Host to connect to as client, or host to listen from as server
     */
    void initSocket(const unsigned int &port, const std::string &host = "");

    /**
     * Initialize a socket connection without resolving host/port
     *
     * \throws range_error No more available IP's to try 
     * \throws logic_error host/port resolution has not been completed yet
     * \throws runtime_error if unable to create socket instance
     */
     void initSocket();

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

    struct addrinfo *servInfo = nullptr;
    struct addrinfo *nextServ = nullptr;

    timeval timeout;
};

} //namespace cloudmutex
#endif
