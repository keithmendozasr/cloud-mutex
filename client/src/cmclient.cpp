#include <cstring>
#include <csignal>
#include <new>

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/configurator.h>
#include <log4cplus/helpers/loglog.h>

#include "client/cmclient.h"

using namespace std;
using namespace log4cplus;

namespace cloudmutex
{

CmClient::CmClient(const std::string &lockName) : lockName(lockName), logger(Logger::getInstance("CmClient"))
{}

const bool CmClient::init()
{
    Logger logger = Logger::getInstance(__PRETTY_FUNCTION__);
    LOG4CPLUS_TRACE(logger, "Initializing "<<lockName<<" lock");
    bool retVal=false;
    try
    {
        if(socket.initClient(9876, "localhost"))
        {
            LOG4CPLUS_DEBUG(logger, "Connected to server");
            socket.writeData("Hello there",strlen("Hello there"));
            sleep(3);
            char buf[256];
            int r = socket.readData(buf, 255);
            buf[r]='\0';
            LOG4CPLUS_INFO(logger, buf);

            retVal = true;
        }
        else
            LOG4CPLUS_FATAL(logger, "Failed to connect to server");
    }
    catch(exception &e)
    {
        LOG4CPLUS_FATAL(logger, "Failed to initialize Mutex. Error message: "<<e.what());
    }
    
    return std::move(retVal);
}

} //namespace cloudmutex
