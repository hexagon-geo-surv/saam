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

TEST(counted_borrow_test, sequential_borrow)
{
    auto process_text = [](saam::ref<std::string> text) { ++(text->at(0)); };

    saam::var<std::string> text("Hello world");

    {
        process_text(text);
        ASSERT_EQ(text.borrow()->at(0), 'I');
    }

    {
        process_text(text);
        ASSERT_EQ(text.borrow()->at(0), 'J');
    }
}

TEST(counted_borrow_test, parallel_borrow)
{
    saam::var<std::string> text("Hello world");

    saam::ref<std::string> text_mut1 = text;
    text_mut1->at(0) = 'Y';
    ASSERT_EQ(text_mut1->at(0), 'Y');

    saam::ref<const std::string> text_immut1 = text;
    ASSERT_EQ(text_immut1->at(0), 'Y');

    saam::ref<const std::string> text_immut2 = text;
    ASSERT_EQ(text_immut2->at(0), 'Y');
}

TEST(counted_borrow_test, dereferencing)
{
    saam::var<int> number(22);

    saam::ref<int> number_mut = number;
    // Assign an lvalue ref
    *number_mut = 23;
    ASSERT_EQ(*number.borrow(), 23);

    // Assign an rvalue ref
    *number.borrow() = 24;
    ASSERT_EQ(*number.borrow(), 24);
}

TEST(counted_borrow_test, nullable_ref)
{
    saam::var<std::string> text("Hello world");

    std::optional<saam::ref<std::string>> maybe_text_ref = text;

    ASSERT_TRUE(maybe_text_ref);
    ASSERT_EQ(maybe_text_ref.value()->at(0), 'H');
}

TEST(counted_borrow_test, var_implicit_borrow)
{
    auto process_text = [](saam::ref<std::string> text) { text->at(0) = 'Y'; };

    saam::var<std::string> text("Hello world");

    saam::ref<std::string> text_ref = text;
    saam::ref<const std::string> text_const_ref = text;
    ASSERT_EQ(text_const_ref->at(0), 'H');
}

TEST(counted_borrow_test, borrow_move_copy_construction)
{
    saam::var<std::string> text("Hello world");

    saam::ref<const std::string> text_ref(text);

    saam::ref<const std::string> copy_text(text_ref);
    ASSERT_EQ(copy_text->at(0), 'H');

    saam::ref<const std::string> moved_text(std::move(text_ref));
    ASSERT_EQ(moved_text->at(0), 'H');
}

TEST(counted_borrow_test, borrow_move_copy_different_instance_assignment)
{
    saam::var<std::string> text("Hello world");
    saam::var<std::string> text2("Welcom world");

    saam::ref<const std::string> textref1(text);
    saam::ref<const std::string> textref2(text2);
    textref2 = textref1;
    ASSERT_EQ(textref2->at(0), 'H');

    textref2 = std::move(textref1);
    ASSERT_EQ(textref2->at(0), 'H');
}

TEST(counted_borrow_test, borrow_move_copy_same_instance_assignment)
{
    saam::var<std::string> text("Hello world");

    saam::ref<const std::string> textref1(text);
    saam::ref<const std::string> textref2 = textref1;
    ASSERT_EQ(textref2->at(0), 'H');

    textref2 = std::move(textref1);
    ASSERT_EQ(textref2->at(0), 'H');
}

TEST(counted_borrow_test, moving_instance)
{
    saam::var<std::string> text("Hello world");

    saam::ref<std::string> text_ref(text);

    std::string text_moved = std::move(*text_ref);

    ASSERT_EQ(text_moved, "Hello world");
    ASSERT_TRUE(text_ref->empty());
}

TEST(counted_borrow_test, comparison)
{
    saam::var<std::string> text("Hello world");
    saam::var<std::string> text2("Welcome world");

    saam::ref<std::string> text1_ref(text);
    saam::ref<std::string> text1_ref2(text);
    saam::ref<std::string> text3_ref(text2);

    ASSERT_TRUE(text1_ref == text1_ref2);
    ASSERT_FALSE(text1_ref != text1_ref2);

    ASSERT_FALSE(text1_ref == text3_ref);
    ASSERT_TRUE(text1_ref != text3_ref);
}

}  // namespace saam::test
