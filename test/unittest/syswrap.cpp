#include <string>
#include <gtest/gtest.h>
#include "syswrap.h"

using namespace std;

SELECT_MODE selectMode;
READ_MODE readMode;
WRITE_MODE writeMode;
char *writeData = nullptr;
int writeSize;
SOCKET_MODE socketMode;

void makeFail(const string &msg, const unsigned int &line)
{
    ADD_FAILURE_AT(__FILE__, line)<<msg;
}

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

int __wrap_socket(int domain, int type, int protocol)
{
    if(socketMode == SOCKET_MODE::FAIL)
    {
        errno = ENFILE;
        return -1;
    }

    return __real_socket(domain, type, protocol);
}
