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
using namespace cloudmutex;

int main(void)
{
    //TODO: Make this configurable
    PropertyConfigurator::doConfigure("/etc/cloud-mutex/clientlog.properties");
    helpers::LogLog::getLogLog()->setInternalDebugging(true);
    Logger logger = Logger::getRoot();
    
    LOG4CPLUS_DEBUG(logger, "Staring client");

    CmClient client("tester");
    if(client.init())
    {
        LOG4CPLUS_INFO(logger, "Initialization complete");
        if(client.lock())
            LOG4CPLUS_INFO(logger, "Lock acquired");
        else
            LOG4CPLUS_ERROR(logger, "Failed to acquire lock");
    }
    else
        LOG4CPLUS_INFO(logger, "Initialization failed");
        
    return 0;
}
