cmake_minimum_required(VERSION 3.16)

find_package(Catch2)

add_executable(ring_buffer_basic_test ring_buffer_basic_test.cc)
add_test(test_ring_buffer_basic ring_buffer_basic_test)
target_link_libraries(ring_buffer_basic_test
                          jmonticelli::ring_buffer
                          Catch2::Catch2WithMain
)

find_package(Boost QUIET)
if (Boost_FOUND)

    set (CMAKE_BUILD_TYPE Release)

    add_executable(ring_buffer_comparitive_benchmark
                       ring_buffer_comparitive_benchmark.cc
    )
    add_test(test_ring_buffer_comparitive_benchmark
                 ring_buffer_comparitive_benchmark
    )
    target_link_libraries(ring_buffer_comparitive_benchmark
                              jmonticelli::ring_buffer
                              Catch2::Catch2WithMain
    )

endif ()
