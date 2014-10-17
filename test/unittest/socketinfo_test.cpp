#include "gtest/gtest.h"
#include <stdexcept>
#include <string>

#include "socketinfo.h"

namespace cloudmutex
{

void getAddrInfoInstance(struct addrinfo **tmp)
{

    struct addrinfo hints;    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    ASSERT_EQ(0, getaddrinfo("localhost", "9876", &hints, tmp));
}

TEST(SocketInfoTest, paramConstructor)
{
    ASSERT_THROW(SocketInfo(2, NULL, 0), std::invalid_argument);
    ASSERT_THROW(SocketInfo(3, NULL, 0), std::invalid_argument);

    SCOPED_TRACE("paramConstructor");
    struct addrinfo *servInfo = nullptr;
    getAddrInfoInstance(&servInfo);
    ASSERT_NO_THROW(SocketInfo(4, (const struct sockaddr_storage *)servInfo->ai_addr, servInfo->ai_addrlen));
    freeaddrinfo(servInfo);
}

TEST(SocketInfoTest, getSocket)
{
    SocketInfo i;
    ASSERT_THROW(i.getSocket(), std::logic_error);

    i.sockfd=500;
    ASSERT_NO_THROW(i.getSocket());
    ASSERT_EQ(500, i.getSocket());
}

TEST(SocketInfoTest, getSocketIP)
{
    SocketInfo i;
    ASSERT_THROW(i.getSocketIP(), std::logic_error);

    struct addrinfo *tmp = nullptr;
    getAddrInfoInstance(&tmp);
    i.sockfd = 500;
    i.setAddrInfo((const struct sockaddr_storage *)tmp->ai_addr, tmp->ai_addrlen);
    std::string rslt;
    ASSERT_NO_THROW(i.getSocketIP());
    ASSERT_STREQ("::1", i.getSocketIP().c_str());
    i.sockfd = 500;
}

} //namespace cloudmutex

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
