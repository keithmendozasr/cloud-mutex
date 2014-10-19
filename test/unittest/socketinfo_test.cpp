#include "gtest/gtest.h"
#include <stdexcept>
#include <string>

#include <log4cplus/nullappender.h>

#include "socketinfo.h"
#include "timeoutexception.h"

using namespace log4cplus;

extern "C"
{
    int __wrap_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
} //extern "C"

enum SELECT_MODE { TIMEOUT, FAIL, READY };
enum SELECT_MODE selectMode;
int __wrap_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exeptfds, struct timeval *timeout)
{
    FD_CLR(500, readfds);

    switch(selectMode)
    {
    case TIMEOUT:
        return 0;
        break;
    case FAIL:
        errno = EBADF;
        return -1;
        break;
    default:
        FD_SET(500, readfds);
        break;
    }

    return 1;
}

namespace cloudmutex
{

void getAddrInfoInstance(struct addrinfo **tmp, const int &ai_family = AF_UNSPEC)
{

    struct addrinfo hints;    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = ai_family;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    ASSERT_EQ(0, getaddrinfo("localhost", "9876", &hints, tmp));
}

class SocketInfoTest : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        //So the log4cplus doesn't complain
        Logger logger = Logger::getRoot();
        logger.removeAllAppenders();
        logger.addAppender(SharedAppenderPtr(new NullAppender));
    }
};

TEST_F(SocketInfoTest, paramConstructor)
{
    ASSERT_THROW(SocketInfo(2, NULL, 0), std::invalid_argument);
    ASSERT_THROW(SocketInfo(3, NULL, 0), std::invalid_argument);

    SCOPED_TRACE("paramConstructor");
    struct addrinfo *servInfo = nullptr;
    getAddrInfoInstance(&servInfo);
    ASSERT_NO_THROW(SocketInfo(4, (const struct sockaddr_storage *)servInfo->ai_addr, servInfo->ai_addrlen));
    freeaddrinfo(servInfo);
}

TEST_F(SocketInfoTest, getSocket)
{
    SocketInfo i;
    ASSERT_THROW(i.getSocket(), std::logic_error);

    i.sockfd=500;
    ASSERT_NO_THROW({
        ASSERT_EQ(500, i.getSocket());
    });
    i.sockfd = -1;
}

TEST_F(SocketInfoTest, getSocketIP)
{
    SocketInfo i;
    ASSERT_THROW(i.getSocketIP(), std::logic_error);
    i.sockfd = -1;

    {
        SocketInfo i;
        i.sockfd = 500;
        struct addrinfo *tmp = nullptr;
        getAddrInfoInstance(&tmp, AF_INET);
        memmove(&(i.addrInfo), tmp->ai_addr, tmp->ai_addrlen);

	    std::unique_ptr<char> buf(new char[INET_ADDRSTRLEN]);
		struct sockaddr_in *t = (struct sockaddr_in *)tmp->ai_addr;
		ASSERT_FALSE(inet_ntop(AF_INET, &(t->sin_addr), buf.get(), INET_ADDRSTRLEN) == nullptr);
		buf.get()[INET_ADDRSTRLEN-1] = '\0';
        freeaddrinfo(tmp);
        std::string retVal = i.getSocketIP();
        i.sockfd = -1;
        ASSERT_NO_THROW({
            ASSERT_STREQ(buf.get(), retVal.c_str());
        });
	}
    
    {
        SocketInfo i;
        i.sockfd = 500;
        struct addrinfo *tmp = nullptr;
        getAddrInfoInstance(&tmp, AF_INET6);
        memmove(&(i.addrInfo), tmp->ai_addr, tmp->ai_addrlen);

		std::unique_ptr<char> buf = std::unique_ptr<char>(new char[INET6_ADDRSTRLEN]);
		struct sockaddr_in6 *t = (struct sockaddr_in6 *)tmp->ai_addr;
		ASSERT_FALSE(inet_ntop(AF_INET6, &(t->sin6_addr), buf.get(), INET6_ADDRSTRLEN) == nullptr);
        freeaddrinfo(tmp);
        std::string retVal = i.getSocketIP();
        i.sockfd=-1;
        ASSERT_NO_THROW({
            ASSERT_STREQ(buf.get(), retVal.c_str()); //i.getSocketIP().c_str());
        });
    }
}

TEST_F(SocketInfoTest, waitForReading)
{

    SocketInfo i;
    i.sockfd=500;

    selectMode = TIMEOUT;
    ASSERT_THROW(i.waitForReading(), TimeoutException);

    selectMode = FAIL;
    ASSERT_THROW(i.waitForReading(), std::system_error);

    selectMode = READY;
    ASSERT_NO_THROW(i.waitForReading());

    i.sockfd = -1;
}

} //namespace cloudmutex
