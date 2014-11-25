#include <gtest/gtest.h>

#include <log4cplus/logger.h>
#include <log4cplus/nullappender.h>
/* For the random occasion that I want to see the log output
#include <log4cplus/consoleappender.h>*/

#include "serversocket.h"

using namespace log4cplus;
using namespace std;
namespace cloudmutex
{

class ServerSocketTest : public ::testing::Test
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
};

typedef ServerSocketTest ServerSocketInitServer;
TEST_F(ServerSocketInitServer, FailPortParam)
{
    ServerSocket target;
    ASSERT_THROW(target.initServer(1), logic_error);
}

} //namespace cloudmutex
