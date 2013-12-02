#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <sys/select.h>

#include <log4cplus/logger.h>

#include "common/socketinfo.h"

namespace cloudmutex
{

/**
 * Base class to handle client connections
 */
class ClientHandler
{
public:
    /**
     * Constructor
     */
    explicit ClientHandler();

    /**
     * Terminate handling client
     */
    inline void terminate()
    {
        keepRunning = false;
    }

    /**
     * Handle client connnection specified in socket
     *
     * \parma socket SocketInfo of client connection
     */
    virtual void handle(const SocketInfo &socket);

protected:
    /**
     * Attempt to read from client.
     *
     * \throws std::invalid_argument data parameter is null
     * \throws std::system_error Error encountered while waiting for message to arrive
     * \throws TimeoutException when timeout reached while waiting for data to arrive
     * \param socket Socket descriptor ID
     * \param data Buffer to place received data
     * \param dataSize Size of data
     * \return Number of bytes received
     */
    const size_t readData(const int &socket, char *data, const size_t &dataSize);

    /**
     * Attempt to send to client
     *
     * \throws std::system_error Error encountered while waiting for socket to be ready to send message
     * \throws TimeoutException when timeout reached while waiting for socket to be ready to send message
     * \param socket Socket descriptor ID
     * \param msg Data to send
     * \param msgSize Size of msg
     * \return Number of bytes sent
     */
    const size_t writeData(const int &socket, const char *msg, const size_t &msgSize);

private:
    bool keepRunning;
    log4cplus::Logger logger;
    fd_set readFd, writeFd;
    timeval timeout;
};

}

#endif
