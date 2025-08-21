# SPDX-FileCopyrightText: Leica Geosystems AG
#
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.23)



#
# Project
#
project(saam
        LANGUAGES CXX
        VERSION ${SAAM_VERSION}
)

set(EXPECTED_MINIMAL_CMAKE_CXX_STANDARD 20)
if ((NOT CMAKE_CXX_STANDARD) OR CMAKE_CXX_STANDARD LESS EXPECTED_MINIMAL_CMAKE_CXX_STANDARD)
    message(FATAL_ERROR "${PROJECT_NAME} requires 'CMAKE_CXX_STANDARD' defined, and at least C++${EXPECTED_MINIMAL_CMAKE_CXX_STANDARD} standard")
endif()

add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

#
# Dependencies
#
# find_package(<dependency> REQUIRED)

#
# Compiler
#

set (PRJ_INCLUDE_DIR_STEM ${CMAKE_CURRENT_SOURCE_DIR}/include)
set (PRJ_INCLUDE_DIR ${PRJ_INCLUDE_DIR_STEM}/${PROJECT_NAME})

set (GENERATED_INCLUDE_STEM ${CMAKE_CURRENT_BINARY_DIR}/generated_code/${PROJECT_NAME}/include)
set (GENERATED_INCLUDE_DIR ${GENERATED_INCLUDE_STEM}/${PROJECT_NAME})
file(MAKE_DIRECTORY ${GENERATED_INCLUDE_STEM})

configure_file(${PRJ_INCLUDE_DIR}/version.hpp.in ${GENERATED_INCLUDE_DIR}/version.hpp)
configure_file(${PRJ_INCLUDE_DIR}/safe_ref.hpp.in ${GENERATED_INCLUDE_DIR}/safe_ref.hpp)

file(GLOB_RECURSE API_HEADERS ${PRJ_INCLUDE_DIR}/*.hpp)
file(GLOB_RECURSE GENERATED_API_HEADERS ${GENERATED_INCLUDE_DIR}/*.hpp)

target_sources(${PROJECT_NAME}
    INTERFACE
        # Defining the header base directories helps us to trim off the base path when installing the headers    
        FILE_SET HEADERS
            BASE_DIRS
                ${PRJ_INCLUDE_DIR_STEM}
                ${GENERATED_INCLUDE_STEM}
            FILES
                ${API_HEADERS}
                ${GENERATED_API_HEADERS}
)

target_include_directories(${PROJECT_NAME}
    INTERFACE
        ${PRJ_INCLUDE_DIR_STEM}
        ${GENERATED_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME}
    INTERFACE
    # <dependency>::<dependency>
)

#
# Installation
#
install(TARGETS ${PROJECT_NAME}
    FILE_SET
        HEADERS
    DESTINATION
        include
)

#
# Unit test
#
include(CTest)
if (BUILD_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()
