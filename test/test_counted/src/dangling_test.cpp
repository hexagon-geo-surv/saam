// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#include <saam/safe_ref.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace saam::test
{

TEST(counted_dangling_test, return_dangling_reference)
{
    auto generate_text = []() -> saam::ref<std::string> {
        saam::var<std::string> text("Hello world");
        return text;
    };

    EXPECT_DEATH({ auto generated_text = generate_text(); }, ".*");
}

TEST(counted_dangling_test, return_dangling_const_reference)
{
    auto generate_text = []() -> saam::ref<const std::string> {
        saam::var<std::string> text("Hello world");
        return text;
    };

    EXPECT_DEATH({ auto generated_text = generate_text(); }, ".*");
}

TEST(counted_dangling_test, dangling_ref_outlives_var)
{
    auto dangling_ref_outlives_var = []() {
        // Reference is created and destroyed before the var variable
        std::optional<saam::ref<const std::string>> capitalized_text_ref;
        saam::var<std::string> text{"hello"};
        capitalized_text_ref = text;
    };

    EXPECT_DEATH({ dangling_ref_outlives_var(); }, ".*");
}

TEST(counted_dangling_test, return_dangling_reference_with_return_value_optimization)
{
    auto return_dangling_reference_with_return_value_optimization = []() {
        saam::var<std::string> text{"hello"};

        std::optional<saam::ref<const std::string>> capitalized_text_ref;

        auto capitalize = [&capitalized_text_ref](saam::ref<const std::string> text) {
            // This variable does not exist here, but at the caller side (RVO)
            saam::var<std::string> capitalized_text(*text);
            capitalized_text_ref = capitalized_text;
            return capitalized_text;
        };

        saam::var<std::string> capitalized_text = capitalize(text);
    };

    EXPECT_DEATH({ return_dangling_reference_with_return_value_optimization(); }, ".*");
}

TEST(counted_dangling_test, container_invalidates_reference)
{
    std::vector<saam::var<std::string>> vec;

    vec.reserve(1);

    vec.emplace_back("hello");
    saam::ref<std::string> text_ref = vec.back();

    // Adding a new element reallocates the internal buffer and invalidates the reference
    EXPECT_DEATH({ vec.emplace_back("world"); }, ".*");
}

}  // namespace saam::test
