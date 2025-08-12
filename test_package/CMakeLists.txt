# SPDX-FileCopyrightText: Leica Geosystems AG
#
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.22)

#
# Project
#
project(saam_test_package
    LANGUAGES CXX
)
add_executable(${PROJECT_NAME})

#
# Dependencies
#
find_package(saam REQUIRED)

#
# Compiler
#
target_sources(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

#
# Linker
#
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        saam::saam
)
