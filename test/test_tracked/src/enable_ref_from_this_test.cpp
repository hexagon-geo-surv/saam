// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#include <saam/safe_ref.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <optional>
#include <regex>
#include <utility>

namespace saam::test
{

class tracked_enable_ref_from_this_test : public ::testing::Test
{
  public:
    void SetUp() override
    {
        global_panic_handler.set_panic_action(std::function<void(std::string_view)>());
        global_panic_handler.clear_panic();
    }
};

class my_class : public saam::enable_ref_from_this<my_class>
{
  public:
    std::function<int(int)> generate_callback() const
    {
        return [self = borrow_from_this()](int data) { return self->increase(data); };
    }

  private:
    int increase(int data) const
    {
        return data + increment_;
    }

    int increment_ = 1;
};

TEST_F(tracked_enable_ref_from_this_test, happy_flow)
{
    saam::var<my_class> my_instance(std::in_place);

    auto callback = my_instance.borrow()->generate_callback();

    ASSERT_EQ(int(6), callback(5));

    ASSERT_FALSE(global_panic_handler.is_panic_active());
}

TEST_F(tracked_enable_ref_from_this_test, dangling_ref)
{
    std::function<void(int)> callback;
    {
        saam::var<my_class> my_instance(std::in_place);
        callback = my_instance.borrow()->generate_callback();

        // my_instance is gone at this point, so the callback contains a dangling reference
    }

    ASSERT_TRUE(global_panic_handler.is_panic_active());
    const bool panic_message_is_correct =
        std::regex_match(global_panic_handler.panic_message().data(), std::regex("^Borrow checked variable of type[.\\s\\S]*"));
    ASSERT_TRUE(panic_message_is_correct);
}

class my_class_only_post_constructor : public saam::enable_ref_from_this<my_class_only_post_constructor>
{
  public:
    void post_constructor()
    {
        self_ = borrow_from_this();
    }

    std::optional<saam::ref<my_class_only_post_constructor>> self_;
};

TEST_F(tracked_enable_ref_from_this_test, pre_ref)
{
    {
        saam::var<my_class_only_post_constructor> my_inst;
    }

    ASSERT_TRUE(global_panic_handler.is_panic_active());
    const bool panic_message_is_correct =
        std::regex_match(global_panic_handler.panic_message().data(), std::regex("^Borrow checked variable of type[.\\s\\S]*"));
    ASSERT_TRUE(panic_message_is_correct);
}

class my_class_with_post_constructor_and_pre_destructor
    : public saam::enable_ref_from_this<my_class_with_post_constructor_and_pre_destructor>
{
  public:
    void post_constructor()
    {
        self_ = borrow_from_this();
    }

    void pre_destructor()
    {
        self_.reset();
    }

    std::optional<saam::ref<my_class_with_post_constructor_and_pre_destructor>> self_;
};

TEST_F(tracked_enable_ref_from_this_test, pre2_ref)
{
    {
        saam::var<my_class_with_post_constructor_and_pre_destructor> my_inst;
    }

    ASSERT_FALSE(global_panic_handler.is_panic_active());
}

}  // namespace saam::test
