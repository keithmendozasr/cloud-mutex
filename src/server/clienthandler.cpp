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

void ClientHandler::handle(const SocketInfo &sockParam)
{
    SocketInfo &socket = const_cast<SocketInfo &> (sockParam);
    string ip = socket.getSocketIP();
    LOG4CPLUS_INFO(logger, "Handle client " << ip);

    string msg;
    keepRunning = true;
    while(keepRunning)
    {
        try
        {
            size_t bufSize = 256;
            char buf[bufSize];
            msg.clear();
            do
            {
                bufSize = socket.readData(buf, bufSize);
                LOG4CPLUS_TRACE(logger, "Read "<<bufSize<<" data");
                msg.append(buf, bufSize);
            }while(bufSize != 0);

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

        try
        {
            msg = string("\nEcho back: ") + msg;
            const char *t = msg.c_str();
            int written=0;
            const int &maxLen = msg.length()+1;
            do
            {
                int len = socket.writeData(t+written, maxLen-written);
                if(!len)
                {
                    LOG4CPLUS_WARN(logger, "Failed to send data");
                    break;
                }
                
                LOG4CPLUS_TRACE(logger, "Written "<<len<<". Amount left: "<<(maxLen-written));
                written+=len;
            }while(written <maxLen);
            LOG4CPLUS_DEBUG(logger, "Echo complete");
        }
        catch(TimeoutException &e)
        {
            LOG4CPLUS_WARN(logger, "Timeout encountered while sending message to client");
            break;
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
