# SPDX-FileCopyrightText: Leica Geosystems AG
#
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.15)

#
# Project
#
project(saam-demo
        LANGUAGES CXX
)

set(EXPECTED_MINIMAL_CMAKE_CXX_STANDARD 17)
if ((NOT CMAKE_CXX_STANDARD) OR CMAKE_CXX_STANDARD LESS EXPECTED_MINIMAL_CMAKE_CXX_STANDARD)
    message(FATAL_ERROR "${PROJECT_NAME} requires 'CMAKE_CXX_STANDARD' defined, and at least C++${EXPECTED_MINIMAL_CMAKE_CXX_STANDARD} standard")
endif()

add_executable(${PROJECT_NAME})

#
# Dependencies
#
find_package(saam REQUIRED)

#
# Compiler
#
file(GLOB_RECURSE IMPL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE IMPL_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp)

target_sources(${PROJECT_NAME}
PRIVATE
    ${IMPL_HEADERS}
    ${IMPL_SOURCES}
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
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
)
