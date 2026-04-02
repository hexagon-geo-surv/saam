// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#include <saam/synchronized.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <latch>
#include <string>
#include <thread>
#include <utility>

namespace saam::test
{

TEST(synchronized_test, emplace_creation)
{
    saam::synchronized<std::string> text(std::in_place, "Hello world");
    ASSERT_EQ(text->length(), 11);
}

TEST(synchronized_test, instance_copy_creation)
{
    saam::synchronized<std::string> text("Hello world");
    ASSERT_EQ(text->length(), 11);
}

TEST(synchronized_test, instance_move_creation)
{
    saam::synchronized<std::string> text(std::string("Hello world"));
    ASSERT_EQ(text->length(), 11);
}

TEST(synchronized_test, copy_constructor)
{
    saam::synchronized<std::string> text(std::string("Hello world"));
    saam::synchronized<std::string> text_copied(text);
    ASSERT_EQ(text->length(), 11);
    ASSERT_EQ(text_copied->length(), 11);
}

TEST(synchronized_test, move_constructor)
{
    saam::synchronized<std::string> text(std::string("Hello world"));
    saam::synchronized<std::string> text_moved(std::move(text));
    ASSERT_TRUE(text->empty());
    ASSERT_EQ(text_moved->length(), 11);
}

TEST(synchronized_test, copy_assignment)
{
    saam::synchronized<std::string> text("Hello world");
    saam::synchronized<std::string> text_copy;
    text_copy = text;
    ASSERT_EQ(*text.commence(), "Hello world");
    ASSERT_EQ(*text_copy.commence(), "Hello world");
}

TEST(synchronized_test, move_assignment)
{
    saam::synchronized<std::string> text("Hello world");
    saam::synchronized<std::string> text_moved;
    text_moved = std::move(text);
    ASSERT_TRUE(text->empty());
    ASSERT_EQ(*text_moved.commence(), "Hello world");
}

TEST(synchronized_test, emplacement_from_underlying)
{
    saam::synchronized<std::string> text("Hello world");
    text.emplace("Hi There");
    ASSERT_EQ(*text.commence(), "Hi There");
}

TEST(synchronized_test, access_with_mutable_content)
{
    saam::synchronized<std::string> text("Hello world");

    ASSERT_EQ(text.commence()->at(0), 'H');

    text.commence_mut()->at(0) = 'Y';
    ASSERT_EQ(text.commence()->at(0), 'Y');
}

TEST(synchronized_test, copy_assignment_from_underlying)
{
    saam::synchronized<std::string> text("Hello world");
    std::string text2("Hi There");
    text = text2;
    ASSERT_EQ(text2, "Hi There");
    ASSERT_EQ(*text.commence(), "Hi There");
}

TEST(synchronized_test, move_assignment_from_underlying)
{
    saam::synchronized<std::string> text("Hello world");
    std::string text2("Hi There");
    text = std::move(text2);
    ASSERT_TRUE(text2.empty());
    ASSERT_EQ(*text.commence(), "Hi There");
}

TEST(synchronized_test, content_assignment)
{
    saam::synchronized<std::string> text("Hello world");
    saam::synchronized<std::string> text_copy;
    *text_copy.commence_mut() = *text.commence();
    ASSERT_EQ(*text_copy.commence(), "Hello world");
}

TEST(synchronized_test, commence_all)
{
    saam::synchronized<std::string> text("Hello world");
    saam::synchronized<int> number(42);

    auto [text_guard, number_guard] = commence_all<const std::string, int>(text, number);

    ASSERT_EQ(text_guard->at(0), 'H');

    ASSERT_EQ(*number_guard, 42);
    *number_guard = 43;
    ASSERT_EQ(*number_guard, 43);
}

TEST(synchronized_test, commence_all_with_retry)
{
    saam::synchronized<std::string> text("Hello world");
    saam::synchronized<int> number(42);

    const auto acquisition_delay = std::chrono::milliseconds(100);

    // This thread holds the mutex for a while, so the main thread acquisition does not succeed for the first time.
    std::latch other_thread_started(1);
    std::thread other_thread([&]() {
        auto text_guard = text.commence_mut();
        other_thread_started.count_down();
        std::this_thread::sleep_for(acquisition_delay);
    });

    other_thread_started.wait();

    auto before = std::chrono::steady_clock::now();
    auto [text_guard, number_guard] = commence_all<const std::string, int>(text, number);
    auto after = std::chrono::steady_clock::now();

    const auto acquisition_duration = std::chrono::duration_cast<std::chrono::milliseconds>(after - before);
    ASSERT_GE(acquisition_duration, acquisition_delay);

    other_thread.join();
}

}  // namespace saam::test
