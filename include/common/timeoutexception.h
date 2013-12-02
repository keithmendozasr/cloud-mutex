#ifndef TIMEOUT_EXCEPTION_H
#define TIMEOUT_EXCEPTION_H

#include <string>
#include <exception>

namespace cloudmutex 
{

/**
 * Exception when socket timeout occurs
 */
class TimeoutException : public std::exception
{
public:
    /**
     * Constructor override from std::exception
     */
    explicit TimeoutException(const std::string &what) noexcept : msg(what) 
    {}

    /**
     * Copy constructor
     */
    TimeoutException(const TimeoutException &cpy) noexcept : msg(cpy.msg)
    {}

    /**
     * Assignment operator overload
     */
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
