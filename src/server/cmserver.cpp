#include <cstring>
/*#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>*/
#include <csignal>
#include <new>

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/configurator.h>
#include <log4cplus/helpers/loglog.h>

#include "server/serversocket.h"
#include "server/clienthandler.h"

using namespace std;
using namespace log4cplus;
using namespace cloudmutex;

bool keepRunning = true;
void sigHandler(int sig)
{
    
    if(sig == SIGINT)
    {
        Logger l = Logger::getRoot();    
        LOG4CPLUS_DEBUG(l, "Got SIGINT");
        keepRunning = false;
    }   
}

//SocketInfo && startListener(const string &port, const unsigned int &backlog)
//SocketInfo 
int main(void)
{
    //TODO: Make this configurable
    PropertyConfigurator::doConfigure("/etc/cloud-mutex/serverlog.properties");
    helpers::LogLog::getLogLog()->setInternalDebugging(true);
    Logger logger = Logger::getRoot();
    
    //Setup SIGINT signal handler
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    
    action.sa_handler = sigHandler;
    
    if(sigaction(SIGINT, &action, NULL))
    {
        LOG4CPLUS_FATAL(logger, "Failed to initialize signal handler. Error: "<<strerror(errno));
        return 1;
    }

    LOG4CPLUS_DEBUG(logger, "Staring server");
    
    ServerSocket socket(9876);
    try
    {
        socket.initServer();
    }
    catch(exception &e)
    {
        LOG4CPLUS_FATAL(logger, "Failed to start listening. Error message: "<<e.what());
        return 1;
    }
    
    LOG4CPLUS_INFO(logger, "waiting for connections");
        
    ClientHandler handler;
    while(keepRunning) 
    {
        try
        {
            socket.handleConnection(handler);
        }
        catch(const bad_alloc &e)
        {
            LOG4CPLUS_FATAL(logger, "Unable to allocate memory to handle client connection");
            socket.closeSocket();
            return 1;
        }
    }
    handler.terminate();

    socket.closeSocket();
    return 0;
}
