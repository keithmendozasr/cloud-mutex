#ifndef CLOUDMUTEX_TEST_SYSWRAP
#define CLOUDMUTEX_TEST_SYSWRAP

#include <string>

extern "C"
{
    int __wrap_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
    ssize_t __wrap_read(int fd, void *buf, size_t count);
    ssize_t __wrap_write(int fd, const void *buf, size_t count);
    extern int __real_socket(int domain, int type, int protocol);
    int __wrap_socket(int domain, int type, int protocol);
} //extern "C"

const unsigned int sockfdSeed = 500;

enum class SELECT_MODE { TIMEOUT, FAIL, READY };
extern SELECT_MODE selectMode;

enum class READ_MODE { FAIL, EOT, GOOD };
extern READ_MODE readMode;

enum class WRITE_MODE { FAIL, EOT, GOOD };
extern WRITE_MODE writeMode;
extern char *writeData;
extern int writeSize;

enum class SOCKET_MODE { FAIL, GOOD };
extern SOCKET_MODE socketMode;

void makeFail(const std::string &msg, const unsigned int &line);

#endif
