#ifndef CLIENT_SOCKET_H
#define CLIENT_SOCKET_H

#include <common/socketinfo.h>

namespace cloudmutex
{

class ClientSocket : private SocketInfo
{
public:
    explicit ClientSocket() : logger(log4cplus::Logger::getInstance("ClientSocket"))
    {};

    const bool initClient(const unsigned int &port, const std::string &server);

private:
    log4cplus::Logger logger;
};

} //namespace cloudmutex
#endif
