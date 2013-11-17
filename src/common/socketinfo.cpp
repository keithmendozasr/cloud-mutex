#include <cstring>
#include <arpa/inet.h>
#include <cstring>

#include <log4cplus/loggingmacros.h>

#include "common/socketinfo.h"

using namespace std;
using namespace log4cplus;

namespace cloudmutex
{

SocketInfo::SocketInfo() :
	logger(Logger::getInstance("SocketInfo")),
	sockfd(-1)
{}

void SocketInfo::initSocket(const string &port, const string &host)
{
	if(sockfd != -1)
		throw logic_error("Instance already intialized");
	
    int rv;
    struct addrinfo hints;
	struct addrinfo *servInfoTmp, *addrInfoItem;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

	rv = getaddrinfo((host.size() ? host.c_str() : NULL), port.c_str(), &hints, &servInfoTmp);
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
	
	setAddrInfo((const struct sockaddr_storage *)servInfoTmp->ai_addr);
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

SocketInfo::SocketInfo(const int &sockfd, const sockaddr_storage *addr) : logger(Logger::getInstance("SocketInfo"))
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
	setAddrInfo(addr);
}

void SocketInfo::setAddrInfo(const sockaddr_storage *addr)
{
	memmove(&addrInfo, addr, sizeof(struct sockaddr_storage));
}

}
