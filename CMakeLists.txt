cmake_minimum_required(VERSION 3.10)
project(jmonticelli_ring_buffer)

set (CMAKE_CXX_STANDARD 11)

add_library(ring_buffer INTERFACE include/jmonticelli/ring_buffer.hh)
add_library(jmonticelli::ring_buffer ALIAS ring_buffer)

target_include_directories(ring_buffer
    INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    INTERFACE $<INSTALL_INTERFACE:include>
)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif()

install(TARGETS ring_buffer
        INCLUDES DESTINATION include
)
