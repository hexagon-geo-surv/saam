// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#include <saam/sentinel.hpp>
#include <saam/synchronized.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <utility>

namespace saam::test
{

class sentinel_test : public ::testing::Test
{
  public:
    sentinel_test() :
        text_(std::string("Hello world"))
    {
    }

    saam::synchronized<std::string> text_;
};

TEST_F(sentinel_test, create_immutable_from_synchronized)
{
    saam::sentinel<const std::string> locked_text(text_);
    ASSERT_EQ(locked_text->length(), 11);
}

TEST_F(sentinel_test, create_mutable_from_synchronized)
{
    saam::sentinel<std::string> locked_text(text_);
    locked_text->at(0) = 'Y';
    ASSERT_EQ(locked_text->at(0), 'Y');
}

TEST_F(sentinel_test, dereferencing)
{
    saam::synchronized<int> number(5);

    {
        saam::sentinel<int> locked_number(number);

        // Assign an lvalue ref
        *locked_number = 23;
        ASSERT_EQ(*locked_number, 23);
    }

    // Assign an rvalue ref
    *number.lock_mut() = 24;
    ASSERT_EQ(*number.lock(), 24);
}

TEST_F(sentinel_test, copy_construct_immutable)
{
    saam::sentinel<const std::string> locked_text(text_);
    saam::sentinel<const std::string> locked_text_copy(locked_text);
    ASSERT_EQ(locked_text_copy->length(), 11);
}

TEST_F(sentinel_test, copy_assignment_immutable)
{
    saam::sentinel<const std::string> locked_text(text_);
    saam::sentinel<const std::string> locked_text_copy(locked_text);
    locked_text_copy = locked_text;
    ASSERT_EQ(locked_text_copy->length(), 11);
}

TEST_F(sentinel_test, move_construct_immutable)
{
    saam::sentinel<const std::string> locked_text(text_);
    saam::sentinel<const std::string> locked_text_move(std::move(locked_text));
    ASSERT_EQ(locked_text_move->length(), 11);
}

TEST_F(sentinel_test, move_assignment_immutable)
{
    saam::sentinel<const std::string> locked_text(text_);
    saam::sentinel<const std::string> locked_text_move(locked_text);
    locked_text_move = std::move(locked_text);
    ASSERT_EQ(locked_text_move->length(), 11);
}

TEST_F(sentinel_test, move_construct_mutable)
{
    saam::sentinel<std::string> locked_text(text_);
    saam::sentinel<std::string> locked_text_move(std::move(locked_text));
    ASSERT_EQ(locked_text_move->length(), 11);
}

TEST_F(sentinel_test, move_assignment_mutable)
{
    saam::sentinel<std::string> locked_text(text_);
    saam::synchronized<std::string> other_text;
    saam::sentinel<std::string> locked_text_move(other_text);
    locked_text_move = std::move(locked_text);
    ASSERT_EQ(locked_text_move->length(), 11);
}

}  // namespace saam::test
