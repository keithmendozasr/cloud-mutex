#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

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
    const std::string readData(const int &socket);

private:
    bool keepRunning;
    log4cplus::Logger logger;
};

}

#endif
