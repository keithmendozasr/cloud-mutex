#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

#include <stdexcept>
#include <system_error>

#include <log4cplus/logger.h>

#include "common/socketinfo.h"

namespace cloudmutex
{

class ClientHandler;

class ServerSocket : public SocketInfo
{
public:
    explicit ServerSocket();
    ServerSocket(const int &port);
    
    void initServer(const int &backlog = 5, const int &port = -1);
    void handleConnection(ClientHandler &handler);
    
protected:
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