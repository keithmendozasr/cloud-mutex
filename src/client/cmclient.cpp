#include <cstring>
#include <csignal>
#include <new>

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/configurator.h>
#include <log4cplus/helpers/loglog.h>

#include "client/clientsocket.h"

using namespace std;
using namespace log4cplus;
using namespace cloudmutex;

//SocketInfo && startListener(const string &port, const unsigned int &backlog)
//SocketInfo 
int main(void)
{
    //TODO: Make this configurable
    PropertyConfigurator::doConfigure("/etc/cloud-mutex/clientlog.properties");
    helpers::LogLog::getLogLog()->setInternalDebugging(true);
    Logger logger = Logger::getRoot();
    
    LOG4CPLUS_DEBUG(logger, "Staring client");
    
    ClientSocket socket;
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
        }
        else
            LOG4CPLUS_FATAL(logger, "Failed to connect to server");
    }
    catch(exception &e)
    {
        LOG4CPLUS_FATAL(logger, "Failed to initialize Mutex. Error message: "<<e.what());
        return 1;
    }
    
    return 0;
}
