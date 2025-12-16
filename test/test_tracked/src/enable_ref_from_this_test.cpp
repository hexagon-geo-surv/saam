// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#include <saam/safe_ref.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <optional>
#include <utility>

namespace saam::test
{

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

TEST(tracked_enable_ref_from_this_test, happy_flow)
{
    saam::var<my_class> my_instance(std::in_place);

    auto callback = my_instance.borrow()->generate_callback();

    ASSERT_EQ(int(6), callback(5));
}

TEST(tracked_enable_ref_from_this_test, dangling_ref)
{
    auto dangling_ref = []() {
        std::function<void(int)> callback;

        saam::var<my_class> my_instance(std::in_place);
        callback = my_instance.borrow()->generate_callback();

        // my_instance is gone at this point, so the callback contains a dangling reference
    };

    EXPECT_DEATH(dangling_ref(), ".*");
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

TEST(tracked_enable_ref_from_this_test, pre_ref)
{
    auto owning_self_reference_at_destruction = []() { saam::var<my_class_only_post_constructor> my_inst; };

    EXPECT_DEATH(owning_self_reference_at_destruction(), ".*");
}

class my_class_with_post_constructor_and_pre_destructor :
    public saam::enable_ref_from_this<my_class_with_post_constructor_and_pre_destructor>
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

TEST(tracked_enable_ref_from_this_test, pre2_ref)
{
    saam::var<my_class_with_post_constructor_and_pre_destructor> my_inst;
}

}  // namespace saam::test
