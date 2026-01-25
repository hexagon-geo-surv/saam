// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#include <saam/sentinel.hpp>
#include <saam/synchronized.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <utility>

namespace saam::test
{

class blindfold_test : public ::testing::Test
{
  public:
    blindfold_test() :
        text_(std::string("Hello world"))
    {
    }

    saam::synchronized<std::string> text_;
};

TEST_F(blindfold_test, blindfold_shared_sentinel)
{
    saam::sentinel<const std::string> text_sent(text_);
    saam::sentinel<const std::string>::blindfold sent_bf(text_sent);

    ASSERT_DEATH((void)text_sent->at(0), "");
}

TEST_F(blindfold_test, blindfold_unique_sentinel)
{
    saam::sentinel<std::string> text_sent(text_);
    saam::sentinel<std::string>::blindfold sent_bf2(text_sent);

    ASSERT_DEATH((void)text_sent->at(0), "");
}

TEST_F(blindfold_test, blindfold_shared_destruction_restores_access)
{
    saam::sentinel<const std::string> text_sent(text_);
    {
        saam::sentinel<const std::string>::blindfold sent_bf(text_sent);
    }
    EXPECT_FALSE(text_sent->empty());
}

TEST_F(blindfold_test, blindfold_unique_destruction_restores_access)
{
    saam::sentinel<std::string> text_sent(text_);
    {
        saam::sentinel<std::string>::blindfold sent_bf(text_sent);
    }
    EXPECT_FALSE(text_sent->empty());
}

}  // namespace saam::test
