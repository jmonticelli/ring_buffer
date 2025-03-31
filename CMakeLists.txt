cmake_minimum_required(VERSION 3.10)

file(READ "${CMAKE_CURRENT_LIST_DIR}/VERSION" PROJECT_VERSION_FROM_FILE)
message("${PROJECT_VERSION_FROM_FILE}")

project(jmonticelli_ring_buffer VERSION ${PROJECT_VERSION_FROM_FILE})

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

install(DIRECTORY include/jmonticelli
        DESTINATION include)

include(CMakePackageConfigHelpers)

install(TARGETS ring_buffer
        EXPORT ${PROJECT_NAME}Targets
        INCLUDES DESTINATION include
)

install(
    EXPORT ${PROJECT_NAME}Targets
    NAMESPACE jmonticelli::
    FILE ${PROJECT_NAME}Targets.cmake
    DESTINATION lib/cmake/${PROJECT_NAME}
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION lib/cmake/${PROJECT_NAME}
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake
          ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake
    DESTINATION lib/cmake/${PROJECT_NAME}
)
