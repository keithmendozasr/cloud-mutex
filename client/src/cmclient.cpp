#include <cstring>
#include <csignal>
#include <new>

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/configurator.h>
#include <log4cplus/helpers/loglog.h>

#include "client/cmclient.h"
#include "common/msgtypes.h"

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

const bool CmClient::lock()
{
    LOG4CPLUS_TRACE(logger, "Sending lock message");
    bool response = false;
    Request request = Request::LOCK;
    socket.writeData(reinterpret_cast<const char *>(&request), sizeof(request));
    size_t readLen = socket.readData(reinterpret_cast<char *>(&response), sizeof(response));
    if(readLen != sizeof(response))
        LOG4CPLUS_FATAL(logger, "Failed to retrieve response");
    LOG4CPLUS_TRACE(logger, "Value of response: "<<response);
    return response;
}

} //namespace cloudmutex
