/**
 * Test circular buffer.
 * 
 * @author Yun Hua
 * @version 0.1 2018.07.02
 */

#define BOOST_TEST_MODULE testbuffer
#include <boost/test/included/unit_test.hpp>
#include <system_error>
#include <future>
#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include "../src/buffer.h"

using namespace goiot;

/**
 * Thread safe rand in range [0.0, 1.0]
 * 
 */
static double randsafe(void)
{
    static std::mutex lock;
    static std::default_random_engine dre;
    static std::uniform_real_distribution<double> di(0, 1);
    std::lock_guard<std::mutex> lg(lock);
    return di(dre);
}

/**
 * Thread safe sine sum / count. 
 * It is nearly equal to 1 - cos(1) or about 0.4597
 * 
 */
static double sinsum(double value)
{
    static std::mutex lock2;
    static int num = 0;
    static double newvalue = 0.0;
    std::lock_guard<std::mutex> lg(lock2);
    newvalue += sin(value);
    num++;
    return newvalue / num;
}

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
            double item = randsafe();
            getbuffer().PutItem(item);
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
            std::cout << value << ", " << sinsum(value) << std::endl;
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
    std::future<void *> res2 = std::async(producer, (void *)nullptr);
    std::future<void *> res3 = std::async(producer, (void *)nullptr);

    std::future<void *> res4 = std::async(consumer, (void *)nullptr);
    std::future<void *> res5 = std::async(consumer, (void *)nullptr);
    std::future<void *> res6 = std::async(consumer, (void *)nullptr);

    std::this_thread::sleep_for(std::chrono::seconds(3));
    getbuffer().SetDone();
    if (res1.wait_for(std::chrono::seconds(0)) != std::future_status::deferred ||
    res2.wait_for(std::chrono::seconds(0)) != std::future_status::deferred ||
    res3.wait_for(std::chrono::seconds(0)) != std::future_status::deferred ||
    res4.wait_for(std::chrono::seconds(0)) != std::future_status::deferred ||
    res5.wait_for(std::chrono::seconds(0)) != std::future_status::deferred ||
    res6.wait_for(std::chrono::seconds(0)) != std::future_status::deferred)
    {
        while(res1.wait_for(std::chrono::seconds(0)) != std::future_status::ready &&
        res2.wait_for(std::chrono::seconds(0)) != std::future_status::ready &&
        res3.wait_for(std::chrono::seconds(0)) != std::future_status::ready &&
        res4.wait_for(std::chrono::seconds(0)) != std::future_status::ready &&
        res5.wait_for(std::chrono::seconds(0)) != std::future_status::ready &&
        res6.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
        {
            std::this_thread::yield();
        }
    }
    res1.get();
    res2.get();
    res3.get();
    res4.get();
    res5.get();
    res6.get();
    BOOST_CHECK_CLOSE(0.4597, sinsum(0.0), 0.1); // 0.1%
}

BOOST_AUTO_TEST_SUITE_END()
