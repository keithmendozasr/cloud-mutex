#include <cstring>
#include <arpa/inet.h>
#include <cstring>
#include <system_error>

#include <log4cplus/loggingmacros.h>

#include "common/socketinfo.h"
#include "common/timeoutexception.h"

using namespace std;
using namespace log4cplus;

namespace cloudmutex
{

SocketInfo::SocketInfo() :
	logger(Logger::getInstance("SocketInfo")),
	sockfd(-1)
{}

void SocketInfo::initSocket(const unsigned int &port, const string &host)
{
	if(sockfd != -1)
		throw logic_error("Instance already intialized");

    if(port > 65535)
        throw invalid_argument("port parameter > 65535");
	
    int rv;
    struct addrinfo hints;
	struct addrinfo *servInfoTmp, *addrInfoItem;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

	rv = getaddrinfo((host.size() ? host.c_str() : NULL), to_string(port).c_str(), &hints, &servInfoTmp);
    if (rv != 0)
    {
        throw runtime_error(gai_strerror(rv));
    }
	
    for(addrInfoItem = servInfoTmp; addrInfoItem != NULL; addrInfoItem = addrInfoItem->ai_next) 
    {
        sockfd = socket(addrInfoItem->ai_family, addrInfoItem->ai_socktype, addrInfoItem->ai_protocol);
        if (sockfd == -1) 
        {
            int err = errno;
            switch(err)
            {
            case EISCONN:
            case EMFILE:
            case ENFILE:
                throw runtime_error(string("Failed to get socket fd: ") + strerror(err));
            default:
                continue;
            }
        }
        else
        {
            LOG4CPLUS_DEBUG(logger, "Socket ready");
            break;
        }
    }
	
    FD_ZERO(&readFd);
    FD_SET(getSocket(), &readFd);
    FD_ZERO(&writeFd);
    FD_SET(getSocket(), &writeFd);

    timeout.tv_sec=15;
    timeout.tv_usec=0;

	setAddrInfo((const struct sockaddr_storage *)servInfoTmp->ai_addr, servInfoTmp->ai_addrlen);
}

const string SocketInfo::getSocketIP() const
{
	if(sockfd == -1)
		throw logic_error("Address info not initialized");
	
    string ip;
	char *buf = nullptr;
	
	if(addrInfo.ss_family == AF_INET)
	{
		buf = new char[INET_ADDRSTRLEN];
		LOG4CPLUS_TRACE(logger, __PRETTY_FUNCTION__<<" ai_family is AF_INET");
		struct sockaddr_in *t = (struct sockaddr_in *)&addrInfo;
		if(inet_ntop(t->sin_family, &(t->sin_addr), buf, INET_ADDRSTRLEN) == nullptr)
		{
			delete[] buf;
			string msg("Failed to translate IP address: ");
			throw msg;
		}
		buf[INET_ADDRSTRLEN] = '\0';
	}
	else if(addrInfo.ss_family == AF_INET6)
	{
		buf = new char[INET6_ADDRSTRLEN];
		LOG4CPLUS_TRACE(logger, __PRETTY_FUNCTION__<<" ai_family is AF_INET6");
		struct sockaddr_in6 *t = (struct sockaddr_in6 *)&addrInfo;
		if(inet_ntop(t->sin6_family, &(t->sin6_addr), buf, INET6_ADDRSTRLEN) == nullptr)
		{
			delete[] buf;
			string msg("Failed to translate IP address: ");
			msg += strerror(errno);
			throw msg;
		}
		buf[INET6_ADDRSTRLEN] = '\0';
	}
	else
		throw invalid_argument("Unexpected sa_family value");
	
	ip = buf;
	
	LOG4CPLUS_TRACE(logger, __PRETTY_FUNCTION__<<" Value of ip: "<<ip);
    return ip;
}

SocketInfo::SocketInfo(const int &sockfd, const sockaddr_storage *addr, const size_t &addrSize) : logger(Logger::getInstance("SocketInfo"))
{
	if(sockfd < 3)
	{
		string msg = "sockfd value: ";
		msg += to_string(sockfd);
		throw invalid_argument(msg);
	}
	
	if(!addr)
		throw invalid_argument("addr is null");
	
	this->sockfd = sockfd;
    FD_ZERO(&readFd);
    FD_SET(getSocket(), &readFd);
    FD_ZERO(&writeFd);
    FD_SET(getSocket(), &writeFd);

    timeout.tv_sec=15;
    timeout.tv_usec=0;
	setAddrInfo(addr, addrSize);
}

void SocketInfo::setAddrInfo(const sockaddr_storage *addr, const size_t &addrSize)
{
	memmove(&addrInfo, addr, sizeof(struct sockaddr_storage));
    addrInfoSize = addrSize;
}

const size_t SocketInfo::readData(char *data, const size_t &dataSize)
{
    if(data == nullptr)
        throw invalid_argument("\"data\" parameter is null");

    int retVal = select(sockfd+1, &readFd, NULL, NULL, &timeout);
    if(retVal == -1)
    {
        int err = errno;
        char errmsg[256];
        strerror_r(err, errmsg, 256);
        LOG4CPLUS_WARN(logger, "Error encountered waiting for socket to be ready. Error message: " << errmsg);
        throw system_error(err, system_category(), errmsg);
    }
    else if(retVal == 0)
    {
        throw TimeoutException("Timeout waiting for data from client");
    }
    else if(FD_ISSET(sockfd, &readFd))
    {
        retVal = read(sockfd, data, dataSize);
        if(retVal < 0)
        {
            int err = errno;
            char errmsg[256];
            strerror_r(err, errmsg, 256);
            throw system_error(err, system_category(), errmsg);
        }
    }

    return retVal;
}

const size_t SocketInfo::writeData(const char *msg, const size_t &msgSize)
{
    int retVal = select(sockfd+1, NULL, &writeFd, NULL, &timeout);
    if(retVal == -1)
    {
        int err = errno;
        char errmsg[256];
        strerror_r(err, errmsg, 256);
        throw system_error(err, system_category(), errmsg);
    }
    else if(retVal == 0)
    {
        throw TimeoutException("Timeout waiting for socket to be ready");
    }
    else
    {
        if(FD_ISSET(sockfd, &readFd))
        {
            retVal = write(sockfd, msg, msgSize);
            if(retVal < 0)
            {
                int err = errno;
                char errmsg[256];
                strerror_r(err, errmsg, 256);
                throw system_error(err, system_category(), errmsg);
            }
            else
                LOG4CPLUS_TRACE(logger, "Read "<<retVal<<" bytes");
        }
    }

    return retVal;
}
}
