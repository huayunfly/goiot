/**
 * Test socket service.
 * 
 * @author Yun Hua
 * @version 0.1 2018.07.05
 */

#define BOOST_TEST_MODULE testsocket
#include <boost/test/included/unit_test.hpp>
#include <string>
#include <iostream>
#include "../src/socketservice.h"

using namespace goiot;

BOOST_AUTO_TEST_SUITE(testsock)

BOOST_AUTO_TEST_CASE(test_open)
{
    SocketService s;
    int sock = s.Open("localhost", 8080, AF_INET);
    if (sock > 0)
    {
        s.Close(sock);
    }
    BOOST_CHECK(sock > 0);
}

BOOST_AUTO_TEST_CASE(test_connect)
{
    SocketService s;
    int client_sockfd = s.Connect(80, "www.baidu.com");
    if (client_sockfd > 0)
    {
        s.Close(client_sockfd);
    }
    BOOST_CHECK(client_sockfd > 0);
}

BOOST_AUTO_TEST_CASE(test_server)
{
    SocketService s;
    int server_sockfd = s.Open("localhost", 8080, AF_INET);
    if (server_sockfd == -1)
    {
        BOOST_CHECK(false);
    }
    s.Work(server_sockfd);

}

BOOST_AUTO_TEST_SUITE_END()