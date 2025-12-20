// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#include <saam/synchronized.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <utility>

namespace saam::test
{

class synchronized_test : public ::testing::Test
{
};

TEST_F(synchronized_test, instance_move_creation)
{
    saam::synchronized<std::string> text(std::in_place, std::string("Hello world"));
    ASSERT_EQ(text.lock()->length(), 11);
}

TEST_F(synchronized_test, emplace_creation)
{
    saam::synchronized<std::string> text(std::in_place, "Hello world");
    ASSERT_EQ(text.lock()->length(), 11);
}

TEST_F(synchronized_test, instance_creation)
{
    saam::synchronized<std::string> text("Hello world");
    ASSERT_EQ(text.lock()->length(), 11);
}

TEST_F(synchronized_test, assignment)
{
    saam::synchronized<std::string> text("Hello world");
    saam::synchronized<std::string> text_copy;
    text_copy = text;
    ASSERT_EQ(*text_copy.lock(), "Hello world");
}

TEST_F(synchronized_test, content_assignment)
{
    saam::synchronized<std::string> text("Hello world");
    saam::synchronized<std::string> text_copy;
    *text_copy.lock_mut() = *text.lock();
    ASSERT_EQ(*text_copy.lock(), "Hello world");
}

TEST_F(synchronized_test, access_with_mutable_content)
{
    saam::synchronized<std::string> text(std::in_place, "Hello world");

    ASSERT_EQ(text.lock()->at(0), 'H');

    text.lock_mut()->at(0) = 'Y';
    ASSERT_EQ(text.lock()->at(0), 'Y');
}

}  // namespace saam::test
