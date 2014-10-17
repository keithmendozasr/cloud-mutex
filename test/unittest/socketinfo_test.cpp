#include "gtest/gtest.h"
#include <stdexcept>
#include "socketinfo.h"

namespace cloudmutex
{

TEST(SocketInfoTest, paramConstructor)
{
    ASSERT_THROW(SocketInfo(2, NULL, 0), std::invalid_argument);
    ASSERT_THROW(SocketInfo(3, NULL, 0), std::invalid_argument);

    struct addrinfo hints;    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    struct addrinfo *servInfo = nullptr;

    ASSERT_EQ(0, getaddrinfo("localhost", "65535", &hints, &servInfo));
    ASSERT_NO_THROW(SocketInfo(4, (const struct sockaddr_storage *)servInfo->ai_addr, servInfo->ai_addrlen));
    freeaddrinfo(servInfo);
}

TEST(SocketInfoTest, getSocket)
{
    SocketInfo i;
    ASSERT_THROW(i.getSocket(), std::logic_error);

    i.sockfd=1;
    ASSERT_NO_THROW(
        ASSERT_EQ(1, i.getSocket())
    );
}

}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
