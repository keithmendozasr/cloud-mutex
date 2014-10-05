#ifndef CMCLIENT_H
#define CMCLIENT_H

#include <string>
#include <log4cplus/logger.h>

#include "clientsocket.h"

namespace cloudmutex
{

class CmClient
{
public:
    CmClient(const std::string &lockName);
    const bool init();

private:
    CmClient(){};
    
    ClientSocket socket;
    const std::string lockName;

    log4cplus::Logger logger;
}; //class CmClient 
} //namespace cloudmutex
#endif
