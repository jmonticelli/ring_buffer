/**
 * Copyright (c) 2025 Julian Monticelli.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
 */

#include <jmonticelli/ring_buffer.hh>
#include <boost/circular_buffer.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>

#include <vector>

TEST_CASE("Benchmark construction", "[construction]")
{
    BENCHMARK("boost::circular_buffer<int> construction size 10")
    {
        return boost::circular_buffer<int>(10);
    };

    BENCHMARK("jmonticelli::ring_buffer<int> construction size 10")
    {
        return jmonticelli::ring_buffer<int>(10);
    };

    BENCHMARK("boost::circular_buffer<int> construction size 100")
    {
        return boost::circular_buffer<int>(100);
    };

    BENCHMARK("jmonticelli::ring_buffer<int> construction size 100")
    {
        return jmonticelli::ring_buffer<int>(100);
    };

    BENCHMARK("boost::circular_buffer<int> construction size 1000")
    {
        return boost::circular_buffer<int>(1000);
    };

    BENCHMARK("jmonticelli::ring_buffer<int> construction size 1000")
    {
        return jmonticelli::ring_buffer<int>(1000);
    };

    BENCHMARK("boost::circular_buffer<int> construction size 10000")
    {
        return boost::circular_buffer<int>(10000);
    };

    BENCHMARK("jmonticelli::ring_buffer<int> construction size 10000")
    {
        return jmonticelli::ring_buffer<int>(10000);
    };
}

void benchmark_push_back(const int container_size, const int elements)
{
    auto cb = boost::circular_buffer<int>(container_size);
    auto rb = jmonticelli::ring_buffer<int>(elements);

    BENCHMARK("boost::circular_buffer<int>(" + std::to_string(container_size) + ") push_back " + std::to_string(elements) + " elems")
    {
        for (int i = 0; i < elements; ++i)
        {
            cb.push_back(i);
        }
        return true;
    };

    BENCHMARK("jmonticelli::ring_buffer<int>(" + std::to_string(container_size) + ") push_back " + std::to_string(elements) + " elems")
    {
        for (int i = 0; i < elements; ++i)
        {
            rb.push_back(i);
        }
        return true;
    };
}

void benchmark_push_front(const int container_size, const int elements)
{
    auto cb = boost::circular_buffer<int>(container_size);
    auto rb = jmonticelli::ring_buffer<int>(elements);

    BENCHMARK("boost::circular_buffer<int>(" + std::to_string(container_size) + ") push_front " + std::to_string(elements) + " elems")
    {
        for (int i = 0; i < 100; ++i)
        {
            cb.push_front(i);
        }
        return true;
    };

    BENCHMARK("jmonticelli::ring_buffer<int>(" + std::to_string(container_size) + ") push_front " + std::to_string(elements) + " elems")
    {
        for (int i = 0; i < 100; ++i)
        {
            rb.push_front(i);
        }
        return true;
    };
}

void benchmark_push_back_front(const int container_size, const int elements)
{
    auto cb = boost::circular_buffer<int>(container_size);
    auto rb = jmonticelli::ring_buffer<int>(elements);

    BENCHMARK("boost::circular_buffer<int>(" + std::to_string(container_size) + ") push_back/push_front " + std::to_string(elements) + " elems")
    {
        for (int i = 0; i < elements; i += 2)
        {
            cb.push_back(i);
            cb.push_front(i);
        }
        return true;
    };

    BENCHMARK("jmonticelli::ring_buffer<int>(" + std::to_string(container_size) + ") push_front " + std::to_string(elements) + " elems")
    {
        for (int i = 0; i < elements; i += 2)
        {
            rb.push_back(i);
            rb.push_front(i);
        }
        return true;
    };
}

TEST_CASE("Bench mark push_back")
{
    for (const auto size : std::vector<int>{10, 100, 1000, 10000})
    {
        benchmark_push_back(size, size);
        benchmark_push_back(size, size * 10);
    }
}

TEST_CASE("Bench mark push_front")
{
    for (const auto size : std::vector<int>{10, 100, 1000, 10000})
    {
        benchmark_push_front(size, size);
        benchmark_push_front(size, size * 10);
    }
}

TEST_CASE("Bench mark push_back/push_front")
{
    for (const auto size : std::vector<int>{10, 100, 1000, 10000})
    {
        benchmark_push_back_front(size, size);
        benchmark_push_back_front(size, size * 10);
    }
}


TEST_CASE("Bench mark copy/destroy/etc")
{
    // The ring buffer does not have an assignment operator right now,
    // so I will make pointers to the buffers instead
    // TODO: replace pointer swapping, use assignment operators when it's available
    boost::circular_buffer<int>* cb_ptr = new boost::circular_buffer<int>(100);
    jmonticelli::ring_buffer<int>* rb_ptr = new jmonticelli::ring_buffer<int>(100);

    // NOTE: I am using push_front for circular buffer, not push_back since it is
    //       the fastest way to insert from my previous benchmarks
    BENCHMARK("boost::circular_buffer<int>(100) push_front 250, copy, swap, destroy, and clear 100 times")
    {
        for (int i = 0; i < 100; ++i)
        {
            for (int j = 0; j < 250; ++j)
            {
                cb_ptr->push_front(j);
            }

            auto other_cb = new boost::circular_buffer<int>(*cb_ptr);
            std::swap(cb_ptr, other_cb);
            delete other_cb;

            cb_ptr->clear();
        }
        return cb_ptr;
    };

    BENCHMARK("jmonticelli::ring_buffer<int>(100) push_back 250, copy, swap, destroy, and clear 100 times")
    {
        for (int i = 0; i < 100; ++i)
        {
            for (int j = 0; j < 250; ++j)
            {
                rb_ptr->push_back(j);
            }

            auto other_rb = new jmonticelli::ring_buffer<int>(*rb_ptr);
            std::swap(rb_ptr, other_rb);
            delete other_rb;

            rb_ptr->clear();
        }
        return rb_ptr;
    };

    // Fill 100 elements in each buffer
    for (int i = 0; i < 100; ++i)
    {
        cb_ptr->push_back(i);
        rb_ptr->push_back(i);
    }

    BENCHMARK("boost::circular_buffer<int>(100) copy/destroy optimally inserted full buffer 100 times")
    {
        for (int i = 0; i < 100; ++i)
        {
            auto other_cb = new boost::circular_buffer<int>(*cb_ptr);
            std::swap(cb_ptr, other_cb);
            delete other_cb;
        }
        return cb_ptr;
    };
    BENCHMARK("jmonticelli::ring_buffer<int>(100) copy/destroy optimally inserted full buffer 100 times")
    {
        for (int i = 0; i < 100; ++i)
        {
            auto other_rb = new jmonticelli::ring_buffer<int>(*rb_ptr);
            std::swap(rb_ptr, other_rb);
            delete other_rb;
        }
        return rb_ptr;
    };

    delete cb_ptr;
    delete rb_ptr;
}
