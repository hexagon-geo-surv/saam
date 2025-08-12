// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#include <saam/var.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <utility>

namespace saam::test
{

class tracked_var_test : public ::testing::Test
{
  public:
    void SetUp() override
    {
        global_panic_handler.set_panic_action(std::function<void(std::string_view)>());
        global_panic_handler.clear_panic();
    }
};

TEST_F(tracked_var_test, var_instance_move_creation)
{
    saam::var<std::string> text(std::in_place, std::string("Hello world"));
    ASSERT_EQ(text.borrow()->length(), 11);
    ASSERT_FALSE(global_panic_handler.is_panic_active());
}

TEST_F(tracked_var_test, var_emplace_creation)
{
    saam::var<std::string> text(std::in_place, "Hello world");
    ASSERT_EQ(text.borrow()->length(), 11);
    ASSERT_FALSE(global_panic_handler.is_panic_active());
}

TEST_F(tracked_var_test, var_instance_creation)
{
    saam::var<std::string> text("Hello world");
    ASSERT_EQ(text.borrow()->length(), 11);
    ASSERT_FALSE(global_panic_handler.is_panic_active());
}

TEST_F(tracked_var_test, var_assignment)
{
    saam::var<std::string> text("Hello world");
    saam::var<std::string> text_copy;
    text_copy = text;
    ASSERT_EQ(*text_copy.borrow(), "Hello world");
    ASSERT_FALSE(global_panic_handler.is_panic_active());
}

TEST_F(tracked_var_test, var_content_assignment)
{
    saam::var<std::string> text("Hello world");
    saam::var<std::string> text_copy;
    *text_copy.borrow() = *text.borrow();
    ASSERT_EQ(*text_copy.borrow(), "Hello world");
    ASSERT_FALSE(global_panic_handler.is_panic_active());
}

TEST_F(tracked_var_test, var_access_with_mutable_content)
{
    saam::var<std::string> text(std::in_place, "Hello world");

    ASSERT_EQ(text.borrow()->at(0), 'H');

    text.borrow()->at(0) = 'Y';
    ASSERT_EQ(text.borrow()->at(0), 'Y');
}

TEST_F(tracked_var_test, var_access_with_immutable_content)
{
    saam::var<const std::string> text(std::in_place, "Hello world");

    ASSERT_EQ(text.borrow()->at(0), 'H');

    // text.borrow()->at(0) = 'Y'; // This does not compile
}

TEST_F(tracked_var_test, var_modification)
{
    saam::var<std::string> text(std::in_place, "Hello world");

    *text.borrow() = "Hi everybody";

    ASSERT_EQ(*text.borrow(), "Hi everybody");
}

}  // namespace saam::test
