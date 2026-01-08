// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#include <saam/safe_ref.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <utility>

namespace saam::test
{

TEST(tracked_var_test, var_underlying_type_copy_creation)
{
    std::string text("Hello world");
    saam::var<std::string> text2(text);
    ASSERT_EQ(text, "Hello world");
    ASSERT_EQ(text2, "Hello world");
}

TEST(tracked_var_test, var_underlying_type_move_creation)
{
    std::string text("Hello world");
    saam::var<std::string> text2(std::move(text));
    ASSERT_TRUE(text.empty());
    ASSERT_EQ(text2, "Hello world");
}

TEST(tracked_var_test, var_emplace_creation)
{
    saam::var<std::string> text(std::in_place, "Hello world");
    ASSERT_EQ(text->length(), 11);
}

TEST(tracked_var_test, var_underlying_type_copy_assignment)
{
    saam::var<std::string> text("Hello world");
    std::string text2("Hello");
    text = text2;
    ASSERT_EQ(text, "Hello");
    ASSERT_EQ(text2, "Hello");
}

TEST(tracked_var_test, var_underlying_type_move_assignment)
{
    saam::var<std::string> text("Hello world");
    std::string text2("Hello");
    text = std::move(text2);
    ASSERT_EQ(text, "Hello");
    ASSERT_TRUE(text2.empty());
}

TEST(tracked_var_test, var_underlying_type_emplacement)
{
    saam::var<std::string> text("Hello world");
    text.emplace("Hello");
    ASSERT_EQ(text->length(), 5);
}

TEST(tracked_var_test, compare_var_with_underlying_type)
{
    saam::var<std::string> text("Hello world");
    bool is_equal = (text == "Hello world");
    ASSERT_TRUE(is_equal);
}

TEST(tracked_var_test, var_copy_assignment)
{
    saam::var<std::string> text("Hello world");
    saam::var<std::string> text2("Welcome");
    text2 = text;
    ASSERT_EQ(text, "Hello world");
    ASSERT_EQ(text2, "Hello world");
}

TEST(tracked_var_test, var_move_assignment)
{
    saam::var<std::string> text("Hello world");
    saam::var<std::string> text2("Welcome");
    text2 = std::move(text);
    ASSERT_TRUE(text->empty());
    ASSERT_EQ(text2, "Hello world");
}

TEST(tracked_var_test, var_access_with_mutable_content)
{
    saam::var<std::string> text("Hello world");

    ASSERT_EQ(text->at(0), 'H');

    text->at(0) = 'Y';
    ASSERT_EQ(text->at(0), 'Y');
}

TEST(tracked_var_test, var_access_with_immutable_content)
{
    saam::var<const std::string> text("Hello world");

    ASSERT_EQ(text->at(0), 'H');

    // text->at(0) = 'Y'; // This does not compile
}

}  // namespace saam::test
