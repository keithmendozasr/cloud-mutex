#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include "common/socketinfo.h"

namespace cloudmutex
{

class ClientHandler
{
public:
    inline void terminate()
    {
        keepRunning = false;
    }

    void handle(const SocketInfo &socket);

protected:
    const std::string readData(const int &socket);

private:
    bool keepRunning;
};

}

#endif
