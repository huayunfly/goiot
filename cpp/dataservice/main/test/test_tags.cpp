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
    std::vector<std::string> names;
    for (std::size_t i = 0; i < total; i++)
    {
        names.push_back(std::string("HelloWorld") + std::to_string(i));
    }
    for (auto name : names)
    {
        auto ptr1 = dict1.Insert(name);
        auto ptr2 = dict2.Insert(name);
        BOOST_CHECK(ptr1 != nullptr);
        BOOST_CHECK(ptr2 != nullptr);
        BOOST_CHECK(ptr1->tagid == ptr2->tagid);
    }

    
    auto dict3 = FixedDict(1 << 7);
    // Add, erase and add
    std::vector<std::string> add_rm_names;
    std::vector<std::size_t> old_positons;
    for (std::size_t i = 0; i < 100; i++)
    {
        add_rm_names.push_back(std::string("And&Remove") + std::to_string(i));
    }
    for (auto name : add_rm_names)
    {
        auto ptr = dict3.Insert(name);
        old_positons.push_back(ptr->tagid);
    }
    for (auto pos : old_positons)
    {
        dict3.Erase(pos);
    }
    for (std::size_t i =0; i < add_rm_names.size(); ++i)
    {
        auto ptr = dict3.Insert(add_rm_names.at(i));
        BOOST_CHECK(ptr->tagid == old_positons.at(i));
    }
    // Key repeat error
    BOOST_CHECK_THROW(dict3.Insert("And&Remove0"), FixedDict::KeyRepetition);

}

BOOST_AUTO_TEST_SUITE_END()