/**
 * Test circular buffer.
 * 
 * @author Yun Hua
 * @version 0.1 2018.07.02
 */

#define BOOST_TEST_MODULE testbuffer
#include <boost/test/included/unit_test.hpp>

#include <ctime>
#include <system_error>
#include <future>
#include <iostream>
#include <vector>
#include <chrono>
#include "../src/buffer.h"

using namespace goiot;

static CircularBuffer<double> &getbuffer()
{
    static CircularBuffer<double> buffer(1 << 5);
    return buffer;
}

static void *producer(void *arg)
{
    int localdone = 0;
    while (!localdone)
    {
        try
        {
            getbuffer().PutItem(1.1);
            getbuffer().GetDone(localdone);
        }
        catch (const std::system_error &e)
        {
            std::cout << e.what() << std::endl;
            return nullptr;
        }
    }
    return nullptr;
}

static void *consumer(void *arg)
{
    double value = 0.0;
    while (true)
    {
        try
        {
            getbuffer().GetItem(value);
            std::cout << value << std::endl;
        }
        catch (const std::system_error &e)
        {
            std::cout << e.what() << std::endl;
            return nullptr;
        }
    }
    return nullptr;
}

BOOST_AUTO_TEST_SUITE(testbuffer)

BOOST_AUTO_TEST_CASE(test_buffer)
{
    std::vector<std::future<void *>> result_producers;
    std::vector<std::future<void *>> result_consumers;

    std::future<void *> res1 = std::async(producer, (void *)nullptr);
    //std::future<void *> res2 = std::async(producer, (void *)nullptr);
    //std::future<void *> res3 = std::async(producer, (void *)nullptr);

    //std::future<void *> res4 = std::async(consumer, (void *)nullptr);
    //std::future<void *> res5 = std::async(consumer, (void *)nullptr);
    std::future<void *> res6 = std::async(consumer, (void *)nullptr);

    std::this_thread::sleep_for(std::chrono::seconds(3));
    getbuffer().SetDone();
    if (res1.wait_for(std::chrono::seconds(0)) != std::future_status::deferred ||
    //res2.wait_for(std::chrono::seconds(0)) != std::future_status::deferred ||
    //res3.wait_for(std::chrono::seconds(0)) != std::future_status::deferred ||
    //res4.wait_for(std::chrono::seconds(0)) != std::future_status::deferred ||
    //res5.wait_for(std::chrono::seconds(0)) != std::future_status::deferred ||
    res6.wait_for(std::chrono::seconds(0)) != std::future_status::deferred)
    {
        while(res1.wait_for(std::chrono::seconds(0)) != std::future_status::ready &&
        //res2.wait_for(std::chrono::seconds(0)) != std::future_status::ready &&
        //res3.wait_for(std::chrono::seconds(0)) != std::future_status::ready &&
        //res4.wait_for(std::chrono::seconds(0)) != std::future_status::ready &&
        //res5.wait_for(std::chrono::seconds(0)) != std::future_status::ready &&
        res6.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
        {
            std::this_thread::yield();
        }
    }
    res1.get();
    //res2.get();
    //res3.get();
    //res4.get();
    //res5.get();
    res6.get();
}

BOOST_AUTO_TEST_SUITE_END()
