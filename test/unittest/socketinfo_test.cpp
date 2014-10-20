#include <cerrno>

#include "gtest/gtest.h"
#include <stdexcept>
#include <string>
#include <regex>

#include <log4cplus/nullappender.h>

#include "socketinfo.h"
#include "timeoutexception.h"

using namespace log4cplus;

extern "C"
{
    int __wrap_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
    ssize_t __wrap_read(int fd, void *buf, size_t count);
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

enum class READ_MODE { FAIL, EOT, GOOD };
READ_MODE readMode;
ssize_t __wrap_read(int fd, void *buf, size_t count)
{
    if(readMode == READ_MODE::FAIL)
    {
        errno = EIO;
        return -1;
    }
    else if(readMode == READ_MODE::EOT)
        return 0;

    const char msg[] = "FROM __wrap_read";
    ssize_t retVal = ((sizeof(msg) < count) ? sizeof(msg) : count);
    memmove(buf, msg, retVal);
    return retVal;
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

    virtual void TearDown()
    {
        if(i.sockfd != -1)
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

typedef SocketInfoTest SocketInfoTestparamConstructor;

TEST_F(SocketInfoTestparamConstructor, FailFDParam)
{
    ASSERT_THROW(SocketInfo(2, NULL, 0), std::invalid_argument);
}

TEST_F(SocketInfoTestparamConstructor, FailaddrinfoParam)
{
    ASSERT_THROW(SocketInfo(3, NULL, 0), std::invalid_argument);
}

TEST_F(SocketInfoTestparamConstructor, goodParams)
{
    SCOPED_TRACE("paramConstructor");
    getAddrInfoInstance(&servInfo);
    ASSERT_NO_THROW(SocketInfo(4, (const struct sockaddr_storage *)servInfo->ai_addr, servInfo->ai_addrlen));
}

typedef SocketInfoTest SocketInfoTestgetSocket;

TEST_F(SocketInfoTestgetSocket, NotReady)
{
    ASSERT_THROW(i.getSocket(), std::logic_error);
}

TEST_F(SocketInfoTestgetSocket, SocketReady)
{
    i.sockfd = 500;
    ASSERT_NO_THROW({
        ASSERT_EQ(500, i.getSocket());
    });
}

typedef SocketInfoTest SocketInfoTestgetSocketIP;

TEST_F(SocketInfoTestgetSocketIP, NotReady)
{
    SocketInfo i;
    ASSERT_THROW(i.getSocketIP(), std::logic_error);
}

TEST_F(SocketInfoTestgetSocketIP, V4)
{
    i.sockfd = 500;
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

TEST_F(SocketInfoTestgetSocketIP, V6)
{
    i.sockfd =500;
    getAddrInfoInstance(&servInfo, AF_INET6);
    memmove(&(i.addrInfo), servInfo->ai_addr, servInfo->ai_addrlen);

    std::unique_ptr<char[]> buf(new char[INET6_ADDRSTRLEN]);
    struct sockaddr_in6 *t = (struct sockaddr_in6 *)servInfo->ai_addr;
    ASSERT_FALSE(inet_ntop(AF_INET6, &(t->sin6_addr), buf.get(), INET6_ADDRSTRLEN) == nullptr);
    ASSERT_NO_THROW({
        ASSERT_STREQ(buf.get(), i.getSocketIP().c_str());
    });
}

typedef SocketInfoTest SocketInfoTestwaitForReading;

TEST_F(SocketInfoTestwaitForReading, Timeout)
{
    selectMode = SELECT_MODE::TIMEOUT;
    ASSERT_THROW(i.waitForReading(), TimeoutException);
}

TEST_F(SocketInfoTestwaitForReading, Fail)
{
    selectMode = SELECT_MODE::FAIL;
    ASSERT_THROW(i.waitForReading(), std::system_error);

}

TEST_F(SocketInfoTestwaitForReading, Ready)
{
    selectMode = SELECT_MODE::READY;
    ASSERT_NO_THROW(i.waitForReading());
}

typedef SocketInfoTest SocketInfoTestreadData;

TEST_F(SocketInfoTestreadData, Failed)
{
    readMode = READ_MODE::FAIL;
    char msg[256];
    ASSERT_THROW(i.readData(msg, 255), std::system_error);
}

TEST_F(SocketInfoTestreadData, Timeout)
{
    selectMode = SELECT_MODE::TIMEOUT;
    char msg[256];
    ASSERT_THROW(i.readData(msg, 255), TimeoutException);
}

TEST_F(SocketInfoTestreadData, Disconnect)
{
    selectMode = SELECT_MODE::READY;
    readMode = READ_MODE::EOT;
    char msg[256];

    try
    {
        i.readData(msg, 255);
        FAIL()<<"Exception should have been thrown";
    }
    catch(std::system_error &e)
    {
        ASSERT_EQ(EPIPE, e.code().value());
    }
}

TEST_F(SocketInfoTestreadData, GoodRead)
{
    selectMode = SELECT_MODE::READY;
    readMode = READ_MODE::GOOD;
    char msg[256];
    int rslt;
    ASSERT_NO_THROW({
        rslt = i.readData(msg, 255);
    });
    ASSERT_EQ(17, rslt);
    ASSERT_STREQ("FROM __wrap_read", msg);
}

} //namespace cloudmutex
