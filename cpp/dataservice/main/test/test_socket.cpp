/**
 * Test socket service.
 * 
 * @author Yun Hua
 * @version 0.1 2018.07.05
 */

#define BOOST_TEST_MODULE testsocket
#include <boost/test/included/unit_test.hpp>
#include "../src/socketservice.h"

using namespace goiot;

BOOST_AUTO_TEST_SUITE(testsock)

BOOST_AUTO_TEST_CASE(test_open)
{
    SocketService s;
    int sock = s.Open(8080);
    if (sock >= 0)
    {
        s.Close(sock);
    }
    BOOST_CHECK_GT(sock, 0);
}

BOOST_AUTO_TEST_CASE(test_connect)
{
    SocketService s;
    int sock = s.Connect(80, "www.baidu.com");
    if (sock >= 0)
    {
        s.Close(sock);
    }
    BOOST_CHECK_GT(sock, 0);
}

BOOST_AUTO_TEST_SUITE_END()