#include <string>
#include <cstring>
#include <system_error>
#include <exception>

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>

#include "server/clienthandler.h"
#include "common/timeoutexception.h"

using namespace std;
using namespace log4cplus;

namespace cloudmutex
{

ClientHandler::ClientHandler() : logger(Logger::getInstance("ClientHandler"))
{}

void ClientHandler::handle(const SocketInfo &socket)
{
    string ip = socket.getSocketIP();
    LOG4CPLUS_INFO(logger, "Handle client " << ip);

    FD_ZERO(&readFd);
    FD_SET(socket.getSocket(), &readFd);

    timeout.tv_sec=15;
    timeout.tv_usec=0;

    string msg;
    keepRunning = true;
    while(keepRunning)
    {
        size_t bufSize = 1024;
        char buf[bufSize];

        try
        {
            bufSize = readData(socket.getSocket(), buf, bufSize);
            msg.clear();
            msg.append(buf, bufSize);
            if(msg == "end")
            {
                LOG4CPLUS_INFO(logger, "Client logging out");
                shutdown(socket.getSocket(), SHUT_RDWR);
                break;
            }
            else
                LOG4CPLUS_DEBUG(logger, "Message from client: >>>"<<msg<<"<<<");
        }
        catch(TimeoutException &e)
        {
            LOG4CPLUS_WARN(logger, "Timeout while waiting for message from client");
            break;
        }
        catch(exception &e)
        {
            LOG4CPLUS_ERROR(logger, "Error encountered while reading message from client: "<<e.what());
            break;
        }
    } //while(keepRunning)

    const_cast<SocketInfo &>(socket).closeSocket();
    LOG4CPLUS_DEBUG(logger, __PRETTY_FUNCTION__<<" exiting");
} //void ClientHandler::handle(const SocketInfo &socket)

const size_t ClientHandler::readData(const int &socket, char *data, const size_t &dataSize)
{
    if(data == nullptr)
        throw invalid_argument("\"data\" parameter is null");

    int retVal = select(socket+1, &readFd, NULL, NULL, &timeout);
    if(retVal == -1)
    {
        int err = errno;
        char errmsg[256];
        strerror_r(err, errmsg, 256);
        LOG4CPLUS_WARN(logger, "Error encountered waiting for socket to be ready. Error message: " << errmsg);
        throw system_error(err, system_category(), errmsg);
    }
    else if(retVal == 0)
    {
        throw TimeoutException("Timeout waiting for data from client");
    }
    else if(FD_ISSET(socket, &readFd))
    {
        retVal = read(socket, data, dataSize);
        if(retVal < 0)
        {
            int err = errno;
            char errmsg[256];
            strerror_r(err, errmsg, 256);
            throw system_error(err, system_category(), errmsg);
        }
    }

    return retVal;
}

} //namespace cloudmutex
