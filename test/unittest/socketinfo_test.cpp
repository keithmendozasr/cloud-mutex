#include <cerrno>

#include "gtest/gtest.h"
#include <stdexcept>
#include <string>
#include <regex>

#include <log4cplus/nullappender.h>
/* For the random occasion that I want to see the log output
#include <log4cplus/consoleappender.h>*/

#include "socketinfo.h"
#include "timeoutexception.h"

using namespace std;
using namespace log4cplus;

extern "C"
{
    int __wrap_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
    ssize_t __wrap_read(int fd, void *buf, size_t count);
    ssize_t __wrap_write(int fd, const void *buf, size_t count);
    extern int __real_socket(int domain, int type, int protocol);
    int __wrap_socket(int domain, int type, int protocol);
} //extern "C"

const unsigned int sockfdSeed = 500;

void makeFail(const string &msg, const unsigned int &line)
{
    ADD_FAILURE_AT(__FILE__, line)<<msg;
}

enum class SELECT_MODE { TIMEOUT, FAIL, READY };
SELECT_MODE selectMode;
int __wrap_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exeptfds, struct timeval *timeout)
{
    fd_set *target;
    if(readfds)
        target = readfds;
    else if(writefds)
        target = writefds;
    else
    {
        makeFail("Neither readfds nor writefds is non-null", __LINE__);
        return -1;
    }

    FD_CLR(sockfdSeed, target);

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
        FD_SET(sockfdSeed, target);
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

enum class WRITE_MODE { FAIL, EOT, GOOD };
WRITE_MODE writeMode;
char *writeData = nullptr;
int writeSize;
ssize_t __wrap_write(int fd, const void *buf, size_t count)
{
    int retVal;

    if(writeMode == WRITE_MODE::FAIL)
    {
        errno = EBADF;
        retVal= -1;
    }
    else if(writeMode == WRITE_MODE::EOT)
    {
        errno = EPIPE;
        retVal = -1;
    }
    else
    {
        try
        {
            writeSize = count;
            writeData = new char[writeSize];
            memmove(writeData, buf, writeSize);
            retVal = writeSize;
        }
        catch(bad_alloc &e)
        {
            makeFail("Failed to allocate memory for writeData", __LINE__);
            retVal = -1;
        }
    }

    return retVal;
}

enum class SOCKET_MODE { FAIL, GOOD };
SOCKET_MODE socketMode;
int __wrap_socket(int domain, int type, int protocol)
{
    if(socketMode == SOCKET_MODE::FAIL)
    {
        errno = ENFILE;
        return -1;
    }

    return __real_socket(domain, type, protocol);
}

namespace cloudmutex
{

class SocketInfoTest : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        Logger logger = Logger::getRoot();
        logger.removeAllAppenders();
        logger.addAppender(SharedAppenderPtr(new NullAppender));
        logger.setLogLevel(OFF_LOG_LEVEL);
        /* For the random occasion that I want to see the log output
        logger.addAppender(SharedAppenderPtr(new ConsoleAppender));
        logger.setLogLevel(TRACE_LOG_LEVEL);*/
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

        if(writeData != nullptr)
        {
            delete[] writeData;
            writeData = nullptr;
        }
    }
        
    void getAddrInfoInstance(const int &ai_family = AF_UNSPEC, const bool &loadOnaddrInfo = false, const string &hostname = "example.com", const string &port = "9876")
    {
        struct addrinfo hints;    
        memset(&hints, 0, sizeof hints);
        hints.ai_family = ai_family;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE; // use my IP

        ASSERT_EQ(0, getaddrinfo(hostname.c_str(), port.c_str(), &hints, &servInfo));

        if(loadOnaddrInfo)
            memmove(&(i.addrInfo), servInfo->ai_addr, servInfo->ai_addrlen);
    }

    void seedSockFd()
    {
        i.sockfd = sockfdSeed;
    }

    void initSocket()
    {
        i.initSocket();
    }

    void initSocket(const unsigned int &port, const std::string &host = "")
    {
        i.initSocket(port, host);
    }

    SocketInfo i;
    struct addrinfo *servInfo = nullptr;
};

typedef SocketInfoTest SocketInfoTestparamConstructor;

TEST_F(SocketInfoTestparamConstructor, FailFDParam)
{
    ASSERT_THROW(SocketInfo(2, NULL, 0), invalid_argument);
}

TEST_F(SocketInfoTestparamConstructor, FailaddrinfoParam)
{
    ASSERT_THROW(SocketInfo(3, NULL, 0), invalid_argument);
}

TEST_F(SocketInfoTestparamConstructor, goodParams)
{
    SCOPED_TRACE("paramConstructor");
    getAddrInfoInstance();
    ASSERT_NO_THROW(SocketInfo(4, (const struct sockaddr_storage *)servInfo->ai_addr, servInfo->ai_addrlen));
}

typedef SocketInfoTest SocketInfoTestgetSocket;

TEST_F(SocketInfoTestgetSocket, NotReady)
{
    ASSERT_THROW(i.getSocket(), logic_error);
}

TEST_F(SocketInfoTestgetSocket, SocketReady)
{
    seedSockFd();
    ASSERT_NO_THROW({
        ASSERT_EQ(500, i.getSocket());
    });
}

typedef SocketInfoTest SocketInfoTestgetSocketIP;

TEST_F(SocketInfoTestgetSocketIP, NotReady)
{
    SocketInfo i;
    ASSERT_THROW(i.getSocketIP(), logic_error);
}

TEST_F(SocketInfoTestgetSocketIP, V4)
{
    seedSockFd();
    getAddrInfoInstance(AF_INET, true);

    unique_ptr<char[]> buf(new char[INET_ADDRSTRLEN]);
    struct sockaddr_in *t = (struct sockaddr_in *)servInfo->ai_addr;
    ASSERT_FALSE(inet_ntop(AF_INET, &(t->sin_addr), buf.get(), INET_ADDRSTRLEN) == nullptr);
    buf.get()[INET_ADDRSTRLEN-1] = '\0';
    ASSERT_NO_THROW({
        ASSERT_STREQ(buf.get(), i.getSocketIP().c_str());
    });
}

TEST_F(SocketInfoTestgetSocketIP, V6)
{
    seedSockFd();
    getAddrInfoInstance(AF_INET6, true);

    unique_ptr<char[]> buf(new char[INET6_ADDRSTRLEN]);
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
    ASSERT_THROW(i.waitForReading(), system_error);
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
    ASSERT_THROW(i.readData(msg, 255), system_error);
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
    catch(system_error &e)
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

typedef SocketInfoTest SocketInfoTestwaitForWriting;

TEST_F(SocketInfoTestwaitForWriting, Timeout)
{
    selectMode = SELECT_MODE::TIMEOUT;
    ASSERT_THROW(i.waitForWriting(), TimeoutException);
}

TEST_F(SocketInfoTestwaitForWriting, Fail)
{
    selectMode = SELECT_MODE::FAIL;
    ASSERT_THROW(i.waitForWriting(), system_error);

}

TEST_F(SocketInfoTestwaitForWriting, Ready)
{
    selectMode = SELECT_MODE::READY;
    ASSERT_NO_THROW(i.waitForWriting());
}

typedef SocketInfoTest SocketInfoTestwriteData;

TEST_F(SocketInfoTestwriteData, Failed)
{
    selectMode = SELECT_MODE::READY;
    writeMode = WRITE_MODE::FAIL;
    char msg[]="TEST";
    ASSERT_THROW(i.writeData(msg, sizeof(msg)), system_error);
}

TEST_F(SocketInfoTestwriteData, Timeout)
{
    selectMode = SELECT_MODE::TIMEOUT;
    char msg[]="TEST";
    ASSERT_THROW(i.writeData(msg, sizeof(msg)), TimeoutException);
}

TEST_F(SocketInfoTestwriteData, Disconnect)
{
    selectMode = SELECT_MODE::READY;
    writeMode = WRITE_MODE::EOT;
    char msg[]="TEST";

    try
    {
        i.writeData(msg, sizeof(msg));
        FAIL()<<"Exception should have been thrown";
    }
    catch(system_error &e)
    {
        ASSERT_EQ(EPIPE, e.code().value());
    }
}

TEST_F(SocketInfoTestwriteData, GoodRead)
{
    selectMode = SELECT_MODE::READY;
    writeMode = WRITE_MODE::GOOD;
    const char msg[] = "WRITE DATA";
    size_t rslt;
    ASSERT_NO_THROW({
        rslt = i.writeData(msg, sizeof(msg));
    });
    ASSERT_EQ(sizeof(msg), rslt);
    ASSERT_STREQ(msg, writeData);
}

class SocketInfoTestinitSocketNoParam : public SocketInfoTest
{
protected:
    virtual void TearDown()
    {
        i.servInfo = nullptr;
        SocketInfoTest::TearDown();
    }

    void getAddrInfoInstance(const int &ai_family = AF_UNSPEC, const bool &loadOnservInfo = false, const bool &loadOnnextServ = false)
    {
        SocketInfoTest::getAddrInfoInstance();

        if(loadOnservInfo)
            i.servInfo = servInfo;

        if(loadOnnextServ)
            i.nextServ = servInfo;
    }
};

TEST_F(SocketInfoTestinitSocketNoParam, BadServInfo)
{
    ASSERT_THROW(initSocket(), logic_error);

    try
    {
        initSocket();
    }
    catch(logic_error &e)
    {
        ASSERT_STREQ("Host info not resolved yet", e.what());
    }
    catch(...)
    {
        FAIL()<<"Unexpected exception encountered";
    }
}

TEST_F(SocketInfoTestinitSocketNoParam, BadNextServ)
{
    getAddrInfoInstance(AF_UNSPEC, true);
    ASSERT_THROW(initSocket(), range_error);

    try
    {
        initSocket();
    }
    catch(range_error &e)
    {
        ASSERT_STREQ("No more resolved IP's to try", e.what());
    }
    catch(...)
    {
        FAIL()<<"Unexpected exception encountered";
    }
}

TEST_F(SocketInfoTestinitSocketNoParam, SocketFail)
{
    socketMode = SOCKET_MODE::FAIL;
    getAddrInfoInstance(AF_UNSPEC, true, true);
    ASSERT_THROW(initSocket(), runtime_error);
}

TEST_F(SocketInfoTestinitSocketNoParam, SocketGood)
{
    socketMode = SOCKET_MODE::GOOD;
    getAddrInfoInstance(AF_UNSPEC, true, true);
    ASSERT_NO_THROW(initSocket());

    ASSERT_TRUE(servInfo->ai_next != NULL) << "example.com should have returned at least 2 IP addresses";
    ASSERT_FALSE(i.nextServ == nullptr);
    ASSERT_EQ(servInfo->ai_next, i.nextServ);

    SCOPED_TRACE("Try next address");
    if(i.nextServ->ai_next)
    {
        //Cut off the last address
        struct addrinfo *tmp = i.nextServ->ai_next;
        i.nextServ->ai_next = NULL;
        freeaddrinfo(tmp);
    }

    ASSERT_NO_THROW(initSocket());

    SCOPED_TRACE("Only one IP resolved");
    getAddrInfoInstance();
    freeaddrinfo(servInfo->ai_next);
    servInfo->ai_next = NULL;
    i.servInfo = servInfo;
    i.nextServ = servInfo;

    ASSERT_NO_THROW(initSocket());
    ASSERT_TRUE(i.nextServ == nullptr);
    ASSERT_THROW(initSocket(), range_error);
}

typedef SocketInfoTest SocketInfoTestinitSocket;

TEST_F(SocketInfoTestinitSocket, Initialized)
{
    seedSockFd();
    ASSERT_THROW(initSocket(9876), logic_error);
    try
    {
        initSocket(9876);
    }
    catch(const logic_error &e)
    {
        ASSERT_STREQ("Instance already initialized", e.what());
    }
    catch(...)
    {
        FAIL() <<"Unexpected exception caught";
    }
}

TEST_F(SocketInfoTestinitSocket, BadPort)
{
    ASSERT_THROW(initSocket(70000), invalid_argument);
    try
    {
        initSocket(70000);
    }
    catch(const logic_error &e)
    {
        ASSERT_STREQ("port parameter > 65535", e.what());
    }
    catch(...)
    {
        FAIL() <<"Unexpected exception caught";
    }
}

TEST_F(SocketInfoTestinitSocket, FailedAddress)
{
    ASSERT_THROW(initSocket(9876, "blah"), runtime_error);
}

TEST_F(SocketInfoTestinitSocket, GoodAddress)
{
    ASSERT_NO_THROW(initSocket(9876, "example.com"));
    ASSERT_NE(-1, i.sockfd);
}

} //namespace cloudmutex
