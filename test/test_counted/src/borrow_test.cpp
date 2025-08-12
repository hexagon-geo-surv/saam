// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#include <saam/panic.hpp>
#include <saam/ref.hpp>
#include <saam/var.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <utility>

namespace saam::test
{

class counted_borrow_test : public ::testing::Test
{
  public:
    void SetUp() override
    {
        global_panic_handler.set_panic_action(std::function<void(std::string_view)>());
        global_panic_handler.clear_panic();
    }
};

TEST_F(counted_borrow_test, sequential_borrow)
{
    auto process_text = [](saam::ref<std::string> text) { ++(text->at(0)); };

    saam::var<std::string> text(std::in_place, "Hello world");

    {
        process_text(text);
        ASSERT_EQ(text.borrow()->at(0), 'I');
        ASSERT_FALSE(global_panic_handler.is_panic_active());
    }

    {
        process_text(text);
        ASSERT_EQ(text.borrow()->at(0), 'J');
        ASSERT_FALSE(global_panic_handler.is_panic_active());
    }
}

TEST_F(counted_borrow_test, parallel_borrow)
{
    saam::var<std::string> text(std::in_place, "Hello world");

    saam::ref<std::string> text_mut1 = text;
    ASSERT_FALSE(global_panic_handler.is_panic_active());

    saam::ref<const std::string> text_immut1 = text;
    ASSERT_FALSE(global_panic_handler.is_panic_active());

    saam::ref<const std::string> text_immut2 = text;
    ASSERT_FALSE(global_panic_handler.is_panic_active());
}

TEST_F(counted_borrow_test, var_implicit_borrow)
{
    auto process_text = [](saam::ref<std::string> text) { text->at(0) = 'Y'; };

    saam::var<std::string> text(std::in_place, "Hello world");

    saam::ref<std::string> text_ref = text;
    saam::ref<const std::string> text_const_ref = text;
    ASSERT_FALSE(global_panic_handler.is_panic_active());
}

TEST_F(counted_borrow_test, borrow_move_copy_construction)
{
    {
        saam::var<std::string> text(std::in_place, "Hello world");

        saam::ref<const std::string> text_ref(text);
        ASSERT_FALSE(global_panic_handler.is_panic_active());

        saam::ref<const std::string> copy_text(text_ref);
        ASSERT_FALSE(global_panic_handler.is_panic_active());

        saam::ref<const std::string> moved_text(std::move(text_ref));
        ASSERT_FALSE(global_panic_handler.is_panic_active());
    }
    ASSERT_FALSE(global_panic_handler.is_panic_active());
}

TEST_F(counted_borrow_test, borrow_move_copy_different_instance_assignment)
{
    saam::var<std::string> text(std::in_place, "Hello world");
    saam::var<std::string> text2(std::in_place, "Welcom world");

    saam::ref<const std::string> textref1(text);
    saam::ref<const std::string> textref2(text2);
    textref2 = textref1;
    ASSERT_FALSE(global_panic_handler.is_panic_active());

    textref2 = std::move(textref1);
    ASSERT_FALSE(global_panic_handler.is_panic_active());
}

TEST_F(counted_borrow_test, borrow_move_copy_same_instance_assignment)
{
    saam::var<std::string> text(std::in_place, "Hello world");

    saam::ref<const std::string> textref1(text);
    saam::ref<const std::string> textref2 = textref1;
    ASSERT_FALSE(global_panic_handler.is_panic_active());

    textref2 = std::move(textref1);
    ASSERT_FALSE(global_panic_handler.is_panic_active());
}

}  // namespace saam::test
