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

enum class SELECT_MODE { TIMEOUT, FAIL, READY };
SELECT_MODE selectMode;
int __wrap_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exeptfds, struct timeval *timeout)
{
    FD_CLR(500, readfds);

    switch(selectMode)
    {
    case SELECT_MODE::TIMEOUT:
        return 0;
        break;
    case SELECT_MODE::FAIL:
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
        Logger logger = Logger::getRoot();
        logger.removeAllAppenders();
        logger.addAppender(SharedAppenderPtr(new NullAppender));
        logger.setLogLevel(OFF_LOG_LEVEL);
    }

    virtual void SetUp()
    {
        i.sockfd = 500;
    }

    virtual void TearDown()
    {
        i.sockfd = -1;
        if(servInfo != nullptr)
        {
            freeaddrinfo(servInfo);
            servInfo = nullptr;
        }
    }
        
    SocketInfo i;
    struct addrinfo *servInfo = nullptr;
};

TEST_F(SocketInfoTest, paramConstructorFailFDParam)
{
    ASSERT_THROW(SocketInfo(2, NULL, 0), std::invalid_argument);
}

TEST_F(SocketInfoTest, paramConstructorFailaddrinfoParam)
{
    ASSERT_THROW(SocketInfo(3, NULL, 0), std::invalid_argument);
}

TEST_F(SocketInfoTest, paramConstructor)
{
    SCOPED_TRACE("paramConstructor");
    getAddrInfoInstance(&servInfo);
    ASSERT_NO_THROW(SocketInfo(4, (const struct sockaddr_storage *)servInfo->ai_addr, servInfo->ai_addrlen));
}

TEST_F(SocketInfoTest, getSocketNotReady)
{
    i.sockfd = -1;
    ASSERT_THROW(i.getSocket(), std::logic_error);
}

TEST_F(SocketInfoTest, getSocket)
{
    ASSERT_NO_THROW({
        ASSERT_EQ(500, i.getSocket());
    });
}

TEST_F(SocketInfoTest, getSocketIPNotReady)
{
    SocketInfo i;
    ASSERT_THROW(i.getSocketIP(), std::logic_error);
}

TEST_F(SocketInfoTest, getSocketIPV4)
{
    getAddrInfoInstance(&servInfo, AF_INET);
    memmove(&(i.addrInfo), servInfo->ai_addr, servInfo->ai_addrlen);

    std::unique_ptr<char[]> buf(new char[INET_ADDRSTRLEN]);
    struct sockaddr_in *t = (struct sockaddr_in *)servInfo->ai_addr;
    ASSERT_FALSE(inet_ntop(AF_INET, &(t->sin_addr), buf.get(), INET_ADDRSTRLEN) == nullptr);
    buf.get()[INET_ADDRSTRLEN-1] = '\0';
    ASSERT_NO_THROW({
        ASSERT_STREQ(buf.get(), i.getSocketIP().c_str());
    });
}

TEST_F(SocketInfoTest, getSocketIPV6)
{
    getAddrInfoInstance(&servInfo, AF_INET6);
    memmove(&(i.addrInfo), servInfo->ai_addr, servInfo->ai_addrlen);

    std::unique_ptr<char[]> buf(new char[INET6_ADDRSTRLEN]);
    struct sockaddr_in6 *t = (struct sockaddr_in6 *)servInfo->ai_addr;
    ASSERT_FALSE(inet_ntop(AF_INET6, &(t->sin6_addr), buf.get(), INET6_ADDRSTRLEN) == nullptr);
    ASSERT_NO_THROW({
        ASSERT_STREQ(buf.get(), i.getSocketIP().c_str());
    });
}

TEST_F(SocketInfoTest, waitForReadingTimeout)
{
    selectMode = SELECT_MODE::TIMEOUT;
    ASSERT_THROW(i.waitForReading(), TimeoutException);
}

TEST_F(SocketInfoTest, waitForReadingFail)
{
    selectMode = SELECT_MODE::FAIL;
    ASSERT_THROW(i.waitForReading(), std::system_error);

}

TEST_F(SocketInfoTest, waitForReadingReady)
{
    selectMode = SELECT_MODE::READY;
    ASSERT_NO_THROW(i.waitForReading());
}

} //namespace cloudmutex
