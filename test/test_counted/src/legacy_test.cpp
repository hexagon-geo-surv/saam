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

TEST(counted_legacy_test, mutable_cpp_variable_to_ref)
{
    auto process_text = [](saam::ref<std::string> text) { text->at(0) = 'Y'; };

    std::string text("Hello world");

    process_text(saam::ref<std::string>(text));
    ASSERT_EQ('Y', text.at(0));
}

TEST(counted_legacy_test, mutable_cpp_variable_to_const_ref)
{
    auto process_text = [](saam::ref<const std::string> text) { return text->at(0); };

    std::string text("Hello world");

    ASSERT_EQ('H', process_text(saam::ref<const std::string>(text)));
}

TEST(counted_legacy_test, const_cpp_variable_to_const_ref)
{
    auto process_text = [](saam::ref<const std::string> text) { return text->at(0); };

    const std::string text("Hello world");

    ASSERT_EQ('H', process_text(saam::ref<const std::string>(text)));
}

TEST(counted_legacy_test, mutable_cpp_ref_to_ref)
{
    auto process_text = [](saam::ref<std::string> text) { text->at(0) = 'Y'; };

    std::string text("Hello world");
    std::string &textref = text;

    process_text(saam::ref<std::string>(textref));
    ASSERT_EQ('Y', text.at(0));
}

TEST(counted_legacy_test, mutable_cpp_ref_to_const_ref)
{
    auto process_text = [](saam::ref<const std::string> text) { return text->at(0); };

    std::string text("Hello world");
    std::string &textref = text;

    ASSERT_EQ('H', process_text(saam::ref<const std::string>(textref)));
}

TEST(counted_legacy_test, const_cpp_ref_to_const_ref)
{
    auto process_text = [](saam::ref<const std::string> text) { return text->at(0); };

    const std::string text("Hello world");
    const std::string &textref = text;

    ASSERT_EQ('H', process_text(saam::ref<const std::string>(textref)));
}

TEST(counted_legacy_test, mutable_cpp_var_to_const_ref)
{
    auto process_text = [](saam::ref<const std::string> text) { return text->at(0); };

    std::string text("Hello world");

    ASSERT_EQ('H', process_text(saam::ref<const std::string>(text)));
}

TEST(counted_legacy_test, const_cpp_var_to_const_ref)
{
    auto process_text = [](saam::ref<const std::string> text) { return text->at(0); };

    const std::string text("Hello world");

    ASSERT_EQ('H', process_text(saam::ref<const std::string>(text)));
}

TEST(counted_legacy_test, var_to_cpp_reference_cast)
{
    saam::var<std::string> text("Hello world");

    // Casting to C++ reference is meant only for backwards compatibility with API that do not support saam,
    // therefore it is always an explicit operation is needed to confirm this unsafe step
    std::string &text_cppref = static_cast<std::string &>(text.borrow());
    const std::string &text_const_cppref = static_cast<const std::string &>(text.borrow());
    text_cppref[0] = 'Y';

    ASSERT_EQ(text_const_cppref.at(0), 'Y');
}

TEST(counted_legacy_test, var_const_to_cpp_reference_cast)
{
    saam::var<const std::string> text("Hello world");

    // Casting to C++ reference is meant only for backwards compatibility with API that do not support saam,
    // therefore it is always an explicit operation is needed to confirm this unsafe step
    const std::string &text_cppref = static_cast<const std::string &>(text.borrow());

    ASSERT_EQ(text_cppref.at(0), 'H');
}

}  // namespace saam::test
