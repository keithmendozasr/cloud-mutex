#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

#include <stdexcept>
#include <system_error>

#include <log4cplus/logger.h>

#include "common/socketinfo.h"

namespace cloudmutex
{

class ClientHandler;

/**
 * Server-side socket connection
 */
class ServerSocket : public SocketInfo
{
public:
    /**
     * Constructor
     */
    explicit ServerSocket();

    /**
     * Constructor taking a port number
     */
    ServerSocket(const int &port);
    
    /**
     * Initiate TCP listening
     *
     * \throws std::logic_error when port not provided and not previously set
     * \param backlog Number of backlog connection
     * \param port Port number to listen to
     * \see SocketInfo::initSocket
     * \see ServerSocket::startListener
     */
    void initServer(const int &backlog = 5, const int &port = -1);

    /**
     * Handle a client connection
     *
     * \param handler ClientHandler instance to handle client connection
     * \see ClientHandler
     */
    void handleConnection(ClientHandler &handler);
    
protected:
    /**
     * Start listening for socket connection
     *
     * \throws std::runtime_error when unable to bind or listen on port
     * \param Number of backlog connection
     */
    void startListener(const int &backlog);
    
private:
    log4cplus::Logger logger;
    int listenPort;
    
    inline void throwSystemError(const int &err, const std::string &msg)
    {
        throw std::system_error(err, std::system_category(), msg);
    }
    
}; //class ServerSocket

}; //namespace cloudmutex

#endif
