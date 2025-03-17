/**
 * Copyright (c) 2025 Julian Monticelli.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
 */

#include <jmonticelli/ring_buffer.hh>

#include <catch2/catch_test_macros.hpp>

#include <vector>

using namespace jmonticelli;

TEST_CASE("ring_buffer handles various types, sizes, and is correctly labeled", "[construct]")
{
    struct TestStruct
    {
        double x;
        int y;
        float z;
    };

    // quite an odd "ring buffer"
    ring_buffer<int> a(1);
    REQUIRE(a.size() == 0);
    REQUIRE(a.capacity() == 1);
    REQUIRE(std::is_same<decltype(a)::value_type, int>::value);

    ring_buffer<bool> b(10);
    REQUIRE(b.size() == 0);
    REQUIRE(b.capacity() == 10);
    REQUIRE(std::is_same<decltype(b)::value_type, bool>::value);

    ring_buffer<float> c(100);
    REQUIRE(c.size() == 0);
    REQUIRE(c.capacity() == 100);
    REQUIRE(std::is_same<decltype(c)::value_type, float>::value);

    ring_buffer<double> d(1000);
    REQUIRE(d.size() == 0);
    REQUIRE(d.capacity() == 1000);
    REQUIRE(std::is_same<decltype(d)::value_type, double>::value);

    ring_buffer<TestStruct> e(10000);
    REQUIRE(e.size() == 0);
    REQUIRE(e.capacity() == 10000);
    REQUIRE(std::is_same<decltype(e)::value_type, TestStruct>::value);
}

TEST_CASE("Construct with a class that does not have a default constructor", "[constructor]")
{
    struct TestStruct
    {
        TestStruct() = delete;
        int x;
    };

    REQUIRE_NOTHROW(ring_buffer<TestStruct>(10));
}

TEST_CASE("Construct using a copy constructor", "[constructor]")
{
    ring_buffer<int> a(10);

    // Fuzz input by doing push_front/push_back
    // in sequence.
    // 0,1,2,3,4,5,6,7,8,9 with a front,back will become 8,6,4,2,0,1,3,5,7,9.
    // This is useful for assuring that we are copying in the same order,
    // even though the memory buffer for the copy and the source buffer
    // will be starting at different pointers.
    for (int i = 0; i < 5; ++i)
    {
        a.push_front(2 * i);
        a.push_back((2 * i) + 1);
    }

    const auto b = a;

    REQUIRE(std::equal(a.cbegin(), a.cend(),
                       b.cbegin(), b.cend()));
}

TEST_CASE("Construct using a copy constructor with non-power-of-two size alignment", "[constructor]")
{
    struct TestStruct
    {
        uint8_t c1;
        uint8_t c2;
        uint8_t c3;

        bool operator==(const TestStruct& other) const
        {
            return this->c1 == other.c1
                and this->c2 == other.c2
                and this->c3 == other.c3;
        }
    };

    ring_buffer<TestStruct> a(10);

    // Fuzz input by doing push_front/push_back
    // in sequence.
    // 0,1,2,3,4,5,6,7,8,9 with a front,back will become 8,6,4,2,0,1,3,5,7,9.
    // This is useful for assuring that we are copying in the same order,
    // even though the memory buffer for the copy and the source buffer
    // will be starting at different pointers.
    for (int i = 0; i < 5; ++i)
    {
        const uint8_t even = static_cast<uint8_t>(i * 2);
        const uint8_t odd = even + 1;

        a.push_front(TestStruct{.c1 = even, .c2 = even, .c3 = even});
        a.push_back(TestStruct{.c1 = odd, .c2 = odd, .c3 = odd});
    }

    const auto b = a;

    REQUIRE(std::equal(a.cbegin(), a.cend(),
                       b.cbegin(), b.cend()));
}

TEST_CASE("Insert elements in the back, confirming size and validating at(n)", "[insertion]")
{
    auto three_ring = ring_buffer<int>(3);

    three_ring.push_back(0);
    REQUIRE(three_ring.at(0) == 0);
    REQUIRE_THROWS(three_ring.at(1));
    REQUIRE_THROWS(three_ring.at(2));
    REQUIRE_THROWS(three_ring.at(3));

    three_ring.push_back(1);
    REQUIRE(three_ring.at(0) == 0);
    REQUIRE(three_ring.at(1) == 1);
    REQUIRE_THROWS(three_ring.at(2));
    REQUIRE_THROWS(three_ring.at(3));

    three_ring.push_back(2);
    REQUIRE(three_ring.at(0) == 0);
    REQUIRE(three_ring.at(1) == 1);
    REQUIRE(three_ring.at(2) == 2);
    REQUIRE_THROWS(three_ring.at(3));

    three_ring.push_back(3);
    REQUIRE(three_ring.at(0) == 1);
    REQUIRE(three_ring.at(1) == 2);
    REQUIRE(three_ring.at(2) == 3);
    REQUIRE_THROWS(three_ring.at(3));
}

TEST_CASE("Insert elements in the front, confirming size and validating at(n)", "[insertion]")
{
    auto three_ring = ring_buffer<int>(3);

    three_ring.push_front(0);
    REQUIRE(three_ring.at(0) == 0);
    REQUIRE_THROWS(three_ring.at(1));
    REQUIRE_THROWS(three_ring.at(2));
    REQUIRE_THROWS(three_ring.at(3));

    three_ring.push_front(1);
    REQUIRE(three_ring.at(0) == 1);
    REQUIRE(three_ring.at(1) == 0);
    REQUIRE_THROWS(three_ring.at(2));
    REQUIRE_THROWS(three_ring.at(3));

    three_ring.push_front(2);
    REQUIRE(three_ring.at(0) == 2);
    REQUIRE(three_ring.at(1) == 1);
    REQUIRE(three_ring.at(2) == 0);
    REQUIRE_THROWS(three_ring.at(3));

    three_ring.push_front(3);
    REQUIRE(three_ring.at(0) == 3);
    REQUIRE(three_ring.at(1) == 2);
    REQUIRE(three_ring.at(2) == 1);
    REQUIRE_THROWS(three_ring.at(3));
}

TEST_CASE("Convert a ring buffer into a vector using iterators", "[iterator]")
{
    auto ring_buf = ring_buffer<int>(10);

    for (int i = 0; i < 10; ++i)
    {
        ring_buf.push_back(i);
    }

    std::vector<int> vec(ring_buf.begin(), ring_buf.end());
    REQUIRE(ring_buf.size() == vec.size());

    for (size_t i = 0; i < vec.size(); ++i)
    {
        REQUIRE(ring_buf.at(i) == vec.at(i));
    }
}

TEST_CASE("Test iterator extremes", "[iterator]")
{
    auto ring_buf = ring_buffer<int>(10);
    ring_buf.push_back(1);
    REQUIRE(*ring_buf.begin() == 1);
    REQUIRE_THROWS(*ring_buf.end());
    REQUIRE_THROWS(*std::next(ring_buf.begin()));

    REQUIRE(std::next(ring_buf.begin()) == ring_buf.end());
    REQUIRE(std::next(ring_buf.begin()) != ring_buf.begin());
    REQUIRE(std::prev(ring_buf.end()) == ring_buf.begin());
    REQUIRE(std::prev(ring_buf.end()) != ring_buf.end());
}

TEST_CASE("Test const iterator", "[iterator]")
{
    auto ring_buf = ring_buffer<int>(3);

    ring_buf.push_back(1);
    ring_buf.push_back(2);
    ring_buf.push_back(3);

    REQUIRE(ring_buf.size() == 3);

    std::vector<int> expected{1, 2, 3};
    REQUIRE(std::equal(ring_buf.cbegin(), ring_buf.cend(),
                       expected.cbegin(), expected.cend()));
}

TEST_CASE("Test front and back", "[accessor]")
{
    auto ring_buf = ring_buffer<int>(3);
    REQUIRE_THROWS(ring_buf.front());
    REQUIRE_THROWS(ring_buf.back());

    ring_buf.push_back(1);
    REQUIRE(ring_buf.front() == 1);
    REQUIRE(ring_buf.back() == 1);

    ring_buf.push_back(2);
    REQUIRE(ring_buf.front() == 1);
    REQUIRE(ring_buf.back() == 2);

    ring_buf.push_back(3);
    REQUIRE(ring_buf.front() == 1);
    REQUIRE(ring_buf.back() == 3);

    ring_buf.push_back(4);
    REQUIRE(ring_buf.front() == 2);
    REQUIRE(ring_buf.back() == 4);
}

TEST_CASE("Test element destructors are called", "[element]")
{
    class TestClass
    {
    public:
        TestClass(int* ptr)
            : ptr_(ptr)
        {
        }

        TestClass(const TestClass& other)
        {
            ptr_ = other.ptr_;
        }

        TestClass(TestClass&& other)
        {
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }

        TestClass& operator=(const TestClass& other)
        {
            return *this = TestClass(other);
        }

        TestClass& operator=(TestClass&& other)
        {
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
            return *this;
        }

        ~TestClass()
        {
            if (ptr_)
            {
                *ptr_ += 1;
            }
        }
    private:
        int* ptr_{nullptr};
    };

    int count = 0;

    ring_buffer<TestClass> rb(2);
    rb.emplace_back(&count);
    rb.emplace_back(&count);
    rb.emplace_back(&count);

    REQUIRE(count == 1);
}
