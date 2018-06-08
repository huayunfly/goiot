/**
 * Test tag data structure and its container's functions.
 * 
 * @author Yun Hua
 * @version 0.1 2018.06.07
 */

#define BOOST_TEST_MODULE testtags
#include <boost/test/included/unit_test.hpp>

#include <ctime>
#include "../src/tagdef.h"

using namespace goiot;

BOOST_AUTO_TEST_SUITE(testtags)

BOOST_AUTO_TEST_CASE(test_tag_def)
{
    TagEntry entry;
    BOOST_CHECK(entry.primValue.tagid == 0);
    BOOST_CHECK(entry.primValue.state.quality == GoQuality::Q_NORMAL);
}

BOOST_AUTO_TEST_SUITE_END()