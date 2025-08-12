# SPDX-FileCopyrightText: Leica Geosystems AG
#
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.14)

#
# Check for stacktrace support
#
include(CheckCXXSourceCompiles)

set(CHECK_STACKTRACE_CODE "
#include <version>
#ifdef __cpp_lib_stacktrace
#if __cpp_lib_stacktrace >= 202011L
int main() { return 0; }
#else
#error \"__cpp_lib_stacktrace is defined but does not meet the required version\"
#endif
#else
#error \"__cpp_lib_stacktrace is not defined\"
#endif
")

check_cxx_source_compiles("${CHECK_STACKTRACE_CODE}" HAS_STACKTRACE_SUPPORT)

if(NOT HAS_STACKTRACE_SUPPORT)
    message(WARNING "Stacktrace support is not available. Ensure your compiler and standard library support C++20 stacktrace.")
    return()
endif()

#
# Project
#
project(saam_test_tracked
    LANGUAGES CXX
)
add_executable(${PROJECT_NAME})

#
# Dependencies
#
find_package(GTest REQUIRED)

#
# Compiler
#
file(GLOB_RECURSE TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
target_sources(${PROJECT_NAME}
    PRIVATE
        ${TEST_SOURCES}
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        SAAM_BORROW_CHECKING_MODE=TRACKED
)

#
# Linker
#
set_target_properties(${PROJECT_NAME} PROPERTIES
    LINKER_LANGUAGE CXX
    INTERPROCEDURAL_OPTIMIZATION TRUE
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        saam::saam
        GTest::gtest
        GTest::gmock
        GTest::gmock_main
    
)

if(NOT CMAKE_CROSSCOMPILING)
    # CTEST Timeout in seconds. 0 means no timeout
    set(CTEST_TEST_TIMEOUT 60)

    include(GoogleTest)

    gtest_discover_tests(${PROJECT_NAME}
      DISCOVERY_MODE PRE_TEST
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )

endif()
