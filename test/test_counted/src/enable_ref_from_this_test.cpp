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

class my_class
{
  public:
    void post_constructor(saam::current_borrow_manager_t &borrow_manager)
    {
        borrow_manager_ = &borrow_manager;
    }

    std::function<int(int)> generate_callback() const
    {
        return [self = saam::ref<const my_class>(*this, borrow_manager_)](int data) { return self->increase(data); };
    }

  private:
    int increase(int data) const
    {
        return data + increment_;
    }

    saam::current_borrow_manager_t *borrow_manager_{nullptr};
    int increment_ = 1;
};

TEST(counted_enable_ref_from_this_test, happy_flow)
{
    saam::var<my_class> my_instance;

    auto callback = my_instance.borrow()->generate_callback();

    ASSERT_EQ(int(6), callback(5));
}

TEST(counted_enable_ref_from_this_test, dangling_ref)
{
    auto dangling_ref = []() {
        std::function<void(int)> callback;

        saam::var<my_class> my_instance;
        callback = my_instance.borrow()->generate_callback();

        // my_instance is gone at this point, so the callback contains a dangling reference
    };

    EXPECT_DEATH(dangling_ref(), ".*");
}

class my_class_only_post_constructor
{
  public:
    void post_constructor(saam::current_borrow_manager_t &borrow_manager)
    {
        self_ = saam::ref<my_class_only_post_constructor>(*this, &borrow_manager);
    }

    std::optional<saam::ref<my_class_only_post_constructor>> self_;
};

TEST(counted_enable_ref_from_this_test, self_reference_not_released_before_destruction)
{
    // The smart self reference is not released before destruction. This dangling reference creates a panic at destruction time.
    auto owning_reference_to_self_at_destruction = []() { saam::var<my_class_only_post_constructor> my_inst; };

    EXPECT_DEATH(owning_reference_to_self_at_destruction(), ".*");
}

class my_class_with_post_constructor_and_pre_destructor
{
  public:
    void post_constructor(saam::current_borrow_manager_t &borrow_manager)
    {
        self_ = saam::ref<my_class_with_post_constructor_and_pre_destructor>(*this, &borrow_manager);
    }

    void pre_destructor()
    {
        // Release the self reference before destruction, so that the instance does not contain
        // a reference to self during destruction.
        self_.reset();
    }

    std::optional<saam::ref<my_class_with_post_constructor_and_pre_destructor>> self_;
};

TEST(counted_enable_ref_from_this_test, self_reference_released_before_destruction)
{
    saam::var<my_class_with_post_constructor_and_pre_destructor> my_inst;
}

}  // namespace saam::test
