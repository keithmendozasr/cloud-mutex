#include <string>
#include <cstring>
#include <system_error>
#include <exception>

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>

#include "clienthandler.h"
#include "common/timeoutexception.h"
#include "common/msgtypes.h"

using namespace std;
using namespace log4cplus;

namespace cloudmutex
{

ClientHandler::ClientHandler() : logger(Logger::getInstance("ClientHandler"))
{}

void ClientHandler::handle(const SocketInfo &sockParam)
{
    SocketInfo &socket = const_cast<SocketInfo &> (sockParam);
    string ip = socket.getSocketIP();
    LOG4CPLUS_INFO(logger, "Handle client " << ip);

    keepRunning = true;
    while(keepRunning)
    {
        try
        {
            Request r;
            size_t readLen = socket.readData(reinterpret_cast<char *>(&r), sizeof(r));
            LOG4CPLUS_TRACE(logger, "Read "<<readLen<<" data");
            switch(r)
            {
            case Request::LOCK:
                LOG4CPLUS_INFO(logger, "Lock requested");
                break;
            default:
                LOG4CPLUS_WARN(logger, "Received unexpected code "<<(int)r);
            }
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

        try
        {
            const bool pass = true;
            const size_t maxLen = sizeof(pass);
            LOG4CPLUS_TRACE(logger, "Value of maxLen: "<<maxLen);
            int len = socket.writeData(reinterpret_cast<const char *>(&pass), sizeof(pass));
            if(!len)
            {
                LOG4CPLUS_WARN(logger, "Failed to send data");
                break;
            }
            else
                LOG4CPLUS_DEBUG(logger, "Response sent");
        }
        catch(TimeoutException &e)
        {
            LOG4CPLUS_WARN(logger, "Timeout encountered while sending message to client");
            break;
        }
        catch(system_error &e)
        {
            if(e.code() == generic_category().default_error_condition(EPIPE))
            {
                LOG4CPLUS_INFO(logger, "Client disconnected");
                break;
            }
        }
        catch(exception &e)
        {
            LOG4CPLUS_ERROR(logger, "Error encountered while sending message to client: "<<e.what());
            break;
        }
    } //while(keepRunning)

    const_cast<SocketInfo &>(socket).closeSocket();
    LOG4CPLUS_DEBUG(logger, __PRETTY_FUNCTION__<<" exiting");
} //void ClientHandler::handle(const SocketInfo &socket)

} //namespace cloudmutex
