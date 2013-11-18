#ifndef TIMEOUT_EXCEPTION_H
#define TIMEOUT_EXCEPTION_H

#include <string>
#include <exception>

namespace cloudmutex 
{
class TimeoutException : public std::exception
{
public:
    explicit TimeoutException(const std::string &what) noexcept : msg(what) 
    {}

    TimeoutException(const TimeoutException &cpy) noexcept : msg(cpy.msg)
    {}

    TimeoutException & operator = (const TimeoutException &cpy) noexcept
    {
        msg = cpy.msg;
        return *this;
    }

    virtual const char * what() const noexcept
    {
        return msg.c_str();
    }

    virtual ~TimeoutException() throw()
    {}

private:
    std::string msg;
};

} //namespace cloudmutex

#endif
