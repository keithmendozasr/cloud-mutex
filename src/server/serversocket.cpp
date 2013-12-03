#include <string>
#include <cstring>

#include <unistd.h>
#include <arpa/inet.h>

#include <log4cplus/loggingmacros.h>

#include "server/serversocket.h"
#include "server/clienthandler.h"

using namespace std;

namespace cloudmutex
{

ServerSocket::ServerSocket() :
    logger(log4cplus::Logger::getInstance("ServerSocket"))
{}

ServerSocket::ServerSocket(const int &port) :
    logger(log4cplus::Logger::getInstance("ServerSocket")),
    listenPort(port)
{}

void ServerSocket::initServer(const int &backlog, const int &port)
{
    LOG4CPLUS_DEBUG(logger, __PRETTY_FUNCTION__<<" start");
    if(port == -1 && listenPort == -1)
        throw logic_error("Listening port not provided");
    else if(port != -1)
    {
        LOG4CPLUS_DEBUG(logger, "Using port provided in parameter");
        listenPort = port;
    }
    else
        LOG4CPLUS_DEBUG(logger, "Using stored listen port");
    
    initSocket(listenPort);
    startListener(backlog);
}

void ServerSocket::startListener(const int &backlog)
{
    const int & sockfd = getSocket();
    
    int yes = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        int err = errno;
        throw runtime_error(string("setsockopt ") + strerror(err));
    }

    LOG4CPLUS_DEBUG(logger, "Attempt to listen to IP "<< getSocketIP());
    struct sockaddr_storage sockAddr = getAddrInfo();
    if (::bind(sockfd, (const struct sockaddr *)&sockAddr, ((sockAddr.ss_family == AF_INET) ? sizeof(sockaddr_in) : sizeof(sockaddr_in6))) == -1) 
    {
        int err = errno;
        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);
        throw runtime_error(string("Failed to bind: ") + strerror(err));
    }

    if (listen(sockfd, backlog) == -1)
    {
        int err = errno;
        throw runtime_error(string("Failed to listen: ") + strerror(err));
    }

    LOG4CPLUS_DEBUG(logger, __PRETTY_FUNCTION__<<" Listening on port "<<listenPort);
}

void ServerSocket::handleConnection(ClientHandler &handler)
{
    struct sockaddr_storage clientAddr;
    socklen_t sin_size = sizeof clientAddr;
    
    int new_fd = accept(getSocket(), (struct sockaddr *)&clientAddr, &sin_size);
    if (new_fd == -1)
    {
        int err = errno;
        if(err == EINTR)
        {
            LOG4CPLUS_DEBUG(logger, "Accept returned from interrupt");
            return;
        }
        else
        {
            throwSystemError(err, "Failed to receive client connection");
        }
    }

    LOG4CPLUS_DEBUG(logger, "Initializing SocketInfo for client");
    /*struct sockaddr *sockAddr;
    switch(clientAddr.ss_family)
    {
    case AF_INET:
        sockAddr = reinterpret_cast<struct sockaddr *>(reinterpret_cast<struct sockaddr_in *>(&clientAddr));
        break;
    case AF_INET6:
        sockAddr = reinterpret_cast<struct sockaddr *>(reinterpret_cast<struct sockaddr_in6 *>(&clientAddr));
        break;
    default:
        throw range_error("Unexpected value in clientAddr.ss_family");
    }*/
    
    SocketInfo clientInfo(new_fd, &clientAddr, sin_size);
    string ip = clientInfo.getSocketIP();
    LOG4CPLUS_DEBUG(logger, "Connection from " << ip);
    LOG4CPLUS_DEBUG(logger, "Passing client to handler");
    handler.handle(std::move(clientInfo));
}

} //namespace cloudmutex
