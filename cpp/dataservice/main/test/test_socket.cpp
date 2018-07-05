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

BOOST_AUTO_TEST_CASE(test_connection)
{
    SocketService s;
    BOOST_CHECK_EQUAL(0, s.Open(8080));
}

BOOST_AUTO_TEST_SUITE_END()