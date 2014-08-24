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

    initSocket(port, server);
    const struct sockaddr_storage &addr = getAddrInfo();
    bool retVal = true;

    if(connect(getSocket(), (struct sockaddr *)&addr, getAddrInfoSize()))
    {
        int err = errno;
        char errmsg[256];
        strerror_r(err, errmsg, 256);
        LOG4CPLUS_ERROR(logger, "Failed to connect to server. Error message: "<<errmsg);
        retVal = false;
    }//connect failed
    else
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

    return retVal;
} //const bool init

} //namespace cloudmutex
