#include <string>
#include <cstring>
#include <system_error>

#include <sys/select.h>

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>

#include "server/clienthandler.h"

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

    fd_set rdfs;
    FD_ZERO(&rdfs);
    FD_SET(socket.getSocket(), &rdfs);

    struct timeval timeout;
    timeout.tv_sec=15;
    timeout.tv_usec=0;

    string msg;
    keepRunning = true;
    while(keepRunning)
    {
        int retVal = select(socket.getSocket()+1, &rdfs, NULL, NULL, &timeout);
        if(retVal == -1)
        {
            int err = errno;
            LOG4CPLUS_WARN(logger, "Error encountered waiting for socket to be ready. Error message: " << strerror(err));
            break;
        }
        else if(retVal == 0)
        {
            LOG4CPLUS_WARN(logger, "Timeout waiting for socket to be ready");
            break;
        }
        else
        {
            if(FD_ISSET(socket.getSocket(), &rdfs))
            {
                LOG4CPLUS_DEBUG(logger, "Reading from "<<ip);
                try
                {
                    msg = readData(socket.getSocket());
                    if(msg == "end")
                    {
                        LOG4CPLUS_INFO(logger, "Client logging out");
                        shutdown(socket.getSocket(), SHUT_RDWR);
                        break;
                    }
                    else
                        LOG4CPLUS_DEBUG(logger, "Message from client: >>>"<<msg<<"<<<");
                }
                catch(system_error &e)
                {
                    LOG4CPLUS_ERROR(logger, "Error encountered while receiving data from client. Error message: "<<e.what());
                    break;
                }
            }
        }
    } //while(keepRunning)

    const_cast<SocketInfo &>(socket).closeSocket();
    LOG4CPLUS_DEBUG(logger, __PRETTY_FUNCTION__<<" exiting");
} //void ClientHandler::handle(const SocketInfo &socket)


const string ClientHandler::readData(const int &socket)
{
    const size_t msgSize = 1024;
    char msg[msgSize];
    int readSize = read(socket, msg, msgSize);
    if(readSize < 0)
    {
        int err = errno;
        throw system_error(err, system_category(), strerror(err));
    }

    msg[readSize]='\0';

    return msg;
}

} //namespace cloudmutex
