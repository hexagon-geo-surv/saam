// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#include <saam/guard.hpp>
#include <saam/synchronized.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <utility>

namespace saam::test
{

class guard_test : public ::testing::Test
{
  public:
    guard_test() :
        text_(std::string("Hello world"))
    {
    }

    saam::synchronized<std::string> text_;
};

TEST_F(guard_test, create_immutable_from_synchronized)
{
    saam::guard<const std::string> locked_text(text_);
    ASSERT_EQ(locked_text->length(), 11);
}

TEST_F(guard_test, create_mutable_from_synchronized)
{
    saam::guard<std::string> locked_text(text_);
    locked_text->at(0) = 'Y';
    ASSERT_EQ(locked_text->at(0), 'Y');
}

TEST_F(guard_test, dereferencing)
{
    saam::synchronized<int> number(5);

    {
        saam::guard<int> locked_number(number);

        // Assign an lvalue ref
        *locked_number = 23;
        ASSERT_EQ(*locked_number, 23);
    }

    // Assign an rvalue ref
    *number.commence_mut() = 24;
    ASSERT_EQ(*number.commence(), 24);
}

TEST_F(guard_test, copy_construct_immutable)
{
    saam::guard<const std::string> locked_text(text_);
    saam::guard<const std::string> locked_text_copy(locked_text);
    ASSERT_EQ(locked_text_copy->length(), 11);
}

TEST_F(guard_test, copy_assignment_immutable)
{
    saam::guard<const std::string> locked_text(text_);
    saam::guard<const std::string> locked_text_copy(locked_text);
    locked_text_copy = locked_text;
    ASSERT_EQ(locked_text_copy->length(), 11);
}

TEST_F(guard_test, move_construct_immutable)
{
    saam::guard<const std::string> locked_text(text_);
    saam::guard<const std::string> locked_text_move(std::move(locked_text));
    ASSERT_EQ(locked_text_move->length(), 11);
}

TEST_F(guard_test, move_assignment_immutable)
{
    saam::guard<const std::string> locked_text(text_);
    saam::guard<const std::string> locked_text_move(locked_text);
    locked_text_move = std::move(locked_text);
    ASSERT_EQ(locked_text_move->length(), 11);
}

TEST_F(guard_test, move_construct_mutable)
{
    saam::guard<std::string> locked_text(text_);
    saam::guard<std::string> locked_text_move(std::move(locked_text));
    ASSERT_EQ(locked_text_move->length(), 11);
}

TEST_F(guard_test, move_assignment_mutable)
{
    saam::guard<std::string> locked_text(text_);
    saam::synchronized<std::string> other_text;
    saam::guard<std::string> locked_text_move(other_text);
    locked_text_move = std::move(locked_text);
    ASSERT_EQ(locked_text_move->length(), 11);
}

TEST_F(guard_test, comparison)
{
    saam::synchronized<std::string> text("Hello world");
    saam::synchronized<std::string> text2("Welcome world");

    // Non-const guard can be only one at a time, otherwise there is a deadlock

    // Only const (immutable) guards can be compared
    saam::guard<const std::string> text1_const_sent(text);
    saam::guard<const std::string> text1_const_sent2(text);
    saam::guard<const std::string> text2_const_sent(text2);

    ASSERT_TRUE(text1_const_sent == text1_const_sent2);
    ASSERT_TRUE(text1_const_sent != text2_const_sent);
}

}  // namespace saam::test
