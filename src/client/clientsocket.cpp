#include <string>
#include <stdexcept>
#include <cstring>

#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>

#include <client/clientsocket.h>
#include <log4cplus/loggingmacros.h>
using namespace std;
namespace cloudmutex
{

const bool ClientSocket::initClient(const unsigned &port, const string &server)
{
    //Check that server not empty
    if(!server.length())
        throw invalid_argument("server parameter");

    bool retVal = true;

    try
    {
        initSocket(port, server);
    }
    catch(runtime_error &e)
    {
        LOG4CPLUS_ERROR(logger, "Host/port resolution failed. Cause: " << e.what());
        return false;
    }

    do
    {
        const struct sockaddr_storage &addr = getAddrInfo();

        if(connect(getSocket(), (struct sockaddr *)&addr, getAddrInfoSize()) != 0)
        {
            int err = errno;
            LOG4CPLUS_TRACE(logger, "Value of err: "<<err);
            switch(err)
            {
            case ECONNREFUSED:
            case ETIMEDOUT:
                LOG4CPLUS_TRACE(logger, "Try next returned IP");
                try
                {
                    initSocket();
                }
                catch(runtime_error &e)
                {
                    LOG4CPLUS_ERROR(logger, "Tried all resolved IP addresses.");
                    retVal = false;
                }
                break;
            default:
                char errmsg[256];
                strerror_r(err, errmsg, 256);
                LOG4CPLUS_ERROR(logger, "Failed to connect to server. Error message: "<<errmsg);
                retVal = false;
                break;
            }
        }
        else
        {
            LOG4CPLUS_DEBUG(logger, "Connected to server");
            break;
        }
    }while(retVal);

    if(retVal)
    {
        if(fcntl(getSocket(), F_SETFD, O_NONBLOCK))
        {
            int err = errno;
            char errmsg[256];
            strerror_r(err, errmsg, 256);
            ostringstream msg;
            msg<<"Failed to set socket to non-blocking. Error message: "<<errmsg;
            throwSystemError(err, msg.str());
        }

        fd_set writefd;
        FD_ZERO(&writefd);
        FD_SET(getSocket(), &writefd);

        struct timeval timeout;
        timeout.tv_sec = 15;
        timeout.tv_usec = 0;

        switch(select(getSocket()+1, NULL, &writefd, NULL, &timeout))
        {
            case -1:
            {
                int err = errno;
                char errmsg[256];
                strerror_r(err, errmsg, 256);
                LOG4CPLUS_ERROR(logger, "Failed to wait for server connection. Error message: "<<errmsg);
                retVal = false;
                break;
            }
            case 0:
            {
                LOG4CPLUS_ERROR(logger, "Unable to connect to server. Conection timedout");
                retVal = false;
                break;
            }
            default:
            {
                if(FD_ISSET(getSocket(), &writefd))
                    LOG4CPLUS_DEBUG(logger, "Connection to server ready");
            }
        }//switch select()
    }//wait for connect ready
    else
        LOG4CPLUS_ERROR(logger, "Unable to connect to the server "<<server<<" on port "<<port);

    return retVal;
} //const bool init

} //namespace cloudmutex
