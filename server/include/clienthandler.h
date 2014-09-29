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

private:
    bool keepRunning;
    log4cplus::Logger logger;
};

}

#endif
