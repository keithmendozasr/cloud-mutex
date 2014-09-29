#include <string>
#include <stdexcept>
#include <cstring>

#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>

#include <log4cplus/loggingmacros.h>

#include "common/timeoutexception.h"
#include "clientsocket.h"
using namespace std;
namespace cloudmutex
{

const bool ClientSocket::initClient(const unsigned &port, const string &server)
{
    //Check that server not empty
    if(!server.length())
        throw invalid_argument("server parameter");

    bool retVal = true;
    int err;
    try
    {
        initSocket(port, server);
    }
    catch(system_error &e)
    {
        LOG4CPLUS_ERROR(logger, "Error encountered connecting to server. Cause: "<<e.what());
        return false;
    }
    catch(runtime_error &e)
    {
        LOG4CPLUS_ERROR(logger, "Host/port resolution failed. Cause: " << e.what());
        return false;
    }

    do
    {
        try
        {
            const struct sockaddr_storage &addr = getAddrInfo();
            if(connect(getSocket(), (struct sockaddr *)&addr, getAddrInfoSize()) != 0)
            {
                err = errno;
                if(err == EINPROGRESS)
                {
                    waitForWriting(3);
                    int val;
                    socklen_t len = sizeof(val);
                    if((err = getsockopt(getSocket(), SOL_SOCKET, SO_ERROR, &val, &len)))
                    {
                        char errmsg[256];
                        strerror_r(err, errmsg, 256);
                        LOG4CPLUS_ERROR(logger, "Failed to connect to server. Error message: "<<errmsg);
                        retVal = false;
                    }
                    else if(val == ECONNREFUSED)
                    {
                        LOG4CPLUS_TRACE(logger, "Failed to connect. Try next IP if available");
                        close(getSocket());
                        initSocket();
                    }
                    else
                        break;
                }
                else
                {
                    char errmsg[256];
                    strerror_r(err, errmsg, 256);
                    LOG4CPLUS_ERROR(logger, "Failed to connect to server. Error message: "<<errmsg);
                    retVal = false;
                    break;
                }
            }
            else
                break;
        }
        catch(system_error &e)
        {
            LOG4CPLUS_ERROR(logger, "Error encountered connecting to server. Cause: "<<e.what());
            retVal = false;
            break;
        }
        catch(runtime_error &e)
        {
            LOG4CPLUS_ERROR(logger, "Host/port resolution failed. Cause: " << e.what());
            retVal = false;
            break;
        }
        catch(TimeoutException &e)
        {
            LOG4CPLUS_ERROR(logger, "Timed-out attempting to connect to server. Try next IP");
            close(getSocket());
            initSocket();
        }
    }while(1);

    if(!retVal)
        LOG4CPLUS_ERROR(logger, "Unable to connect to the server "<<server<<" on port "<<port);
    else
        LOG4CPLUS_DEBUG(logger, "Connected to server");

    return retVal;
} //const bool init

} //namespace cloudmutex
