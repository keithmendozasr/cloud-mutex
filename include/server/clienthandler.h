#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include "common/socketinfo.h"

namespace cloudmutex
{

class ClientHandler
{
public:
    void handle(const SocketInfo &socket);
};

}

#endif