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

TEST(unchecked_borrow_test, ref_instance_size)
{
    // In unchecked mode, ref should have the same size as the raw reference/pointer -> no overhead
    ASSERT_EQ(sizeof(saam::ref<std::string>), sizeof(std::string *));
}

TEST(unchecked_borrow_test, sequential_borrow)
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

TEST(unchecked_borrow_test, parallel_borrow)
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

TEST(unchecked_borrow_test, nullable_ref)
{
    saam::var<std::string> text("Hello world");

    std::optional<saam::ref<std::string>> maybe_text_ref = text;

    ASSERT_TRUE(maybe_text_ref);
    ASSERT_EQ(maybe_text_ref.value()->at(0), 'H');
}

TEST(unchecked_borrow_test, var_implicit_borrow)
{
    auto process_text = [](saam::ref<std::string> text) { text->at(0) = 'Y'; };

    saam::var<std::string> text("Hello world");

    saam::ref<std::string> text_ref = text;
    saam::ref<const std::string> text_const_ref = text;
    ASSERT_EQ(text_const_ref->at(0), 'H');
}

TEST(unchecked_borrow_test, borrow_move_copy_construction)
{
    saam::var<std::string> text("Hello world");

    saam::ref<const std::string> text_ref(text);

    saam::ref<const std::string> copy_text(text_ref);
    ASSERT_EQ(copy_text->at(0), 'H');

    saam::ref<const std::string> moved_text(std::move(text_ref));
    ASSERT_EQ(moved_text->at(0), 'H');
}

TEST(unchecked_borrow_test, borrow_move_copy_different_instance_assignment)
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

TEST(unchecked_borrow_test, borrow_move_copy_same_instance_assignment)
{
    saam::var<std::string> text("Hello world");

    saam::ref<const std::string> textref1(text);
    saam::ref<const std::string> textref2 = textref1;
    ASSERT_EQ(textref2->at(0), 'H');

    textref2 = std::move(textref1);
    ASSERT_EQ(textref2->at(0), 'H');
}

TEST(unchecked_borrow_test, comparison)
{
    saam::var<std::string> text("Hello world");
    saam::var<std::string> text2("Welcome world");

    saam::ref<std::string> text_ref(text);
    saam::ref<std::string> text_ref2(text);
    saam::ref<std::string> text2_ref(text2);

    ASSERT_TRUE(text_ref == text_ref2);
    ASSERT_FALSE(text_ref != text_ref2);

    ASSERT_FALSE(text_ref == text2_ref);
    ASSERT_TRUE(text_ref != text2_ref);
}

}  // namespace saam::test
