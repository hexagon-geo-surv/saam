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

add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

#
# Dependencies
#

#
# Compiler
#

message(STATUS "saam borrow checking mode: ${SAAM_BORROW_CHECKING_MODE_CMAKE}")

set (PRJ_INCLUDE_DIR_STEM ${CMAKE_CURRENT_SOURCE_DIR}/include)
set (PRJ_INCLUDE_DIR ${PRJ_INCLUDE_DIR_STEM}/${PROJECT_NAME})

set (GENERATED_INCLUDE_STEM ${CMAKE_CURRENT_BINARY_DIR}/generated_code/${PROJECT_NAME}/include)
set (GENERATED_INCLUDE_DIR ${GENERATED_INCLUDE_STEM}/${PROJECT_NAME})
file(MAKE_DIRECTORY ${GENERATED_INCLUDE_STEM})

configure_file(${PRJ_INCLUDE_DIR}/version.hpp.in ${GENERATED_INCLUDE_DIR}/version.hpp)
configure_file(${PRJ_INCLUDE_DIR}/detail/default_borrow_manager.hpp.in ${GENERATED_INCLUDE_DIR}/detail/default_borrow_manager.hpp)

file(GLOB_RECURSE API_HEADERS ${PRJ_INCLUDE_DIR}/*.hpp ${PRJ_INCLUDE_DIR}/*.ipp)
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

# Define minimum C++ standard - unless it is defined in the toolchain
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)

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
