#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <sys/select.h>

#include <log4cplus/logger.h>

#include "common/socketinfo.h"

namespace cloudmutex
{

class ClientHandler
{
public:
    explicit ClientHandler();

    inline void terminate()
    {
        keepRunning = false;
    }

    void handle(const SocketInfo &socket);

protected:
    const size_t readData(const int &socket, char *msg, const size_t &msgSize);

private:
    bool keepRunning;
    log4cplus::Logger logger;
    fd_set readFd;
    timeval timeout;
};

}

#endif
