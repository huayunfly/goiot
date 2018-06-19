/**
 * Test tag data structure and its container's functions.
 * 
 * @author Yun Hua
 * @version 0.1 2018.06.07
 */

#define BOOST_TEST_MODULE testtags
#include <boost/test/included/unit_test.hpp>

#include <ctime>
#include <stdexcept>
#include "../src/hashtable.h"

using namespace goiot;

BOOST_AUTO_TEST_SUITE(testtags)

BOOST_AUTO_TEST_CASE(test_tag_def)
{
    TagEntry entry;
    BOOST_CHECK(entry.primValue.tagid == 0);
    BOOST_CHECK(entry.primValue.state.quality == GoQuality::Q_NORMAL);
}

BOOST_AUTO_TEST_CASE(test_hashtable_init)
{
    BOOST_CHECK_THROW(FixedDict(0), std::invalid_argument);
    BOOST_CHECK_THROW(FixedDict(FixedDict::DICT_MAXSIZE + 1), std::invalid_argument);
    BOOST_CHECK(FixedDict(FixedDict::DICT_MAXSIZE).Size() == FixedDict::DICT_MAXSIZE);

    // Test slots allocation size algorithm
    std::size_t newsize = 8;
    for (; newsize < (FixedDict::DICT_MAXSIZE / 2 * 3); newsize <<= 1)
        ;
    BOOST_CHECK(FixedDict(FixedDict::DICT_MAXSIZE).ReservedSize() == newsize);
}

BOOST_AUTO_TEST_CASE(test_hashtable)
{
    // Init a fixed dictionary
    std::size_t total = 1 << 15;
    auto dict1 = FixedDict(total);
    auto dict2 = FixedDict(total);


    // Add items in order
    std::vector<std::wstring> names;
    for (std::size_t i = 0; i < total; i++)
    {
        names.push_back(std::wstring(L"HelloWorld") + std::to_wstring(i));
    }
    for (auto name : names)
    {
        auto ptr1 = dict1.AddItem(name);
        auto ptr2 = dict2.AddItem(name);
        BOOST_CHECK(ptr1 != nullptr);
        BOOST_CHECK(ptr2 != nullptr);
        BOOST_CHECK(ptr1->tagid == ptr2->tagid);
    }
}

BOOST_AUTO_TEST_SUITE_END()