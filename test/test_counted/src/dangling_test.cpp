// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#include <saam/var.hpp>
#include <saam/ref.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <optional>
#include <regex>
#include <string>
#include <utility>
#include <vector>

namespace saam::test
{

class counted_dangling_test : public ::testing::Test
{
  public:
    void SetUp() override
    {
        global_panic_handler.set_panic_action(std::function<void(std::string_view)>());
        global_panic_handler.clear_panic();
    }
};

TEST_F(counted_dangling_test, return_dangling_reference)
{
    auto generate_text = []() -> saam::ref<std::string> {
        saam::var<std::string> text(std::in_place, "Hello world");
        return text;
    };

    auto generated_text = generate_text();
    ASSERT_TRUE(global_panic_handler.is_panic_active());
    const bool panic_message_is_correct =
        std::regex_match(global_panic_handler.panic_message().data(), std::regex("^Borrow checked variable of type[.\\s\\S]*"));
    ASSERT_TRUE(panic_message_is_correct);
}

TEST_F(counted_dangling_test, return_dangling_const_reference)
{
    auto generate_text = []() -> saam::ref<const std::string> {
        saam::var<std::string> text(std::in_place, "Hello world");
        return text;
    };

    auto generated_text = generate_text();
    ASSERT_TRUE(global_panic_handler.is_panic_active());
    const bool panic_message_is_correct =
        std::regex_match(global_panic_handler.panic_message().data(), std::regex("^Borrow checked variable of type[.\\s\\S]*"));
    ASSERT_TRUE(panic_message_is_correct);
}

TEST_F(counted_dangling_test, free_variable_before_ref)
{
    {
        // Reference is created and destroyed before the var variable
        std::optional<saam::ref<const std::string>> capitalized_text_ref;

        saam::var<std::string> text{std::in_place, "hello"};

        capitalized_text_ref = text;
    }

    ASSERT_TRUE(global_panic_handler.is_panic_active());
    const bool panic_message_is_correct =
        std::regex_match(global_panic_handler.panic_message().data(), std::regex("^Borrow checked variable of type[.\\s\\S]*"));
    ASSERT_TRUE(panic_message_is_correct);
}

TEST_F(counted_dangling_test, return_dangling_reference_with_return_value_optimization)
{
    {
        saam::var<std::string> text{std::in_place, "hello"};

        // This reference is initialized from the temporal local variable, still will be valid
        // as the returned instance is created at the caller side because of RVO
        std::optional<saam::ref<const std::string>> capitalized_text_ref;

        auto capitalize = [&capitalized_text_ref](saam::ref<const std::string> text) {
            // This variable does not exist here, but at the caller side (RVO)
            saam::var<std::string> capitalized_text(std::in_place, *text);
            capitalized_text_ref = capitalized_text;
            return capitalized_text;
        };

        saam::var<std::string> capitalized_text = capitalize(text);
    }

    ASSERT_TRUE(global_panic_handler.is_panic_active());
    const bool panic_message_is_correct =
        std::regex_match(global_panic_handler.panic_message().data(), std::regex("^Borrow checked variable of type[.\\s\\S]*"));
    ASSERT_TRUE(panic_message_is_correct);
}

TEST_F(counted_dangling_test, container_invalidates_reference)
{
    std::vector<saam::var<std::string>> vec;
    vec.reserve(1);

    vec.emplace_back(std::in_place, "hello");
    saam::ref<std::string> text_ref = vec.back();

    ASSERT_FALSE(global_panic_handler.is_panic_active());

    // Adding a new element reallocates the internal buffer and invalidates the reference
    vec.emplace_back(std::in_place, "world");

    ASSERT_TRUE(global_panic_handler.is_panic_active());
    const bool panic_message_is_correct =
        std::regex_match(global_panic_handler.panic_message().data(), std::regex("^Borrow checked variable of type[.\\s\\S]*"));
    ASSERT_TRUE(panic_message_is_correct);
}

}  // namespace saam::test
