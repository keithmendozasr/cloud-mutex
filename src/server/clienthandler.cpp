#include <string>
#include <cstring>

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>

#include "server/clienthandler.h"

using namespace std;
using namespace log4cplus;

namespace cloudmutex
{

void ClientHandler::handle(const SocketInfo &socket)
{
    Logger logger = Logger::getInstance("ClientHandler");
    string ip = socket.getSocketIP();
    LOG4CPLUS_INFO(logger, "Handle client " << ip);
    
    if (send(socket.getSocket(), "Hello, world!", 13, 0) == -1)
        LOG4CPLUS_ERROR(logger, "Failed to send to client: "<<strerror(errno));

    const_cast<SocketInfo &>(socket).closeSocket();
}

}
