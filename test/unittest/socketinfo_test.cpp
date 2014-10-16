#include "gtest/gtest.h"
#include <stdexcept>
#include "socketinfo.h"

namespace cloudmutex
{

TEST(SocketInfoTest, getSocket)
{
    SocketInfo i;
    ASSERT_THROW(i.getSocket(), std::logic_error);
}

}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
