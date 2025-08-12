// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#include <saam/panic.hpp>
#include <saam/ref.hpp>
#include <saam/var.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

namespace saam::test
{

class counted_casting_test : public ::testing::Test
{
  public:
    void SetUp() override
    {
        global_panic_handler.set_panic_action(std::function<void(std::string_view)>());
        global_panic_handler.clear_panic();
    }
};

class base
{
  public:
    base() = default;
    virtual ~base() = default;
    virtual std::string get_dynamice_name() const
    {
        return "base";
    }
    std::string get_static_name() const
    {
        return "base";
    }
};

class derived : public base
{
  public:
    derived() = default;
    ~derived() override = default;
    std::string get_dynamice_name() const override
    {
        return "derived";
    }
    std::string get_static_name() const
    {
        return "derived";
    }
};

class derived2 : public base
{
  public:
    derived2() = default;
    ~derived2() override = default;
    std::string get_dynamice_name() const override
    {
        return "derived2";
    }
    std::string get_static_name() const
    {
        return "derived2";
    }
};

TEST_F(counted_casting_test, explicit_upcasting_construction_from_var)
{
    saam::var<derived> derived_instance;

    saam::ref<base> base_reference = derived_instance.borrow();
    ASSERT_EQ(base_reference->get_dynamice_name(), "derived");
    ASSERT_EQ(base_reference->get_static_name(), "base");
    ASSERT_FALSE(global_panic_handler.is_panic_active());

    saam::ref<const base> const_base_reference = derived_instance.borrow();
    ASSERT_EQ(const_base_reference->get_dynamice_name(), "derived");
    ASSERT_EQ(const_base_reference->get_static_name(), "base");
    ASSERT_FALSE(global_panic_handler.is_panic_active());
}

TEST_F(counted_casting_test, implicit_upcasting_construction_from_var)
{
    saam::var<derived> derived_instance;

    saam::ref<base> base_reference = derived_instance;
    ASSERT_EQ(base_reference->get_dynamice_name(), "derived");
    ASSERT_EQ(base_reference->get_static_name(), "base");
    ASSERT_FALSE(global_panic_handler.is_panic_active());

    saam::ref<const base> const_base_reference = derived_instance;
    ASSERT_EQ(const_base_reference->get_dynamice_name(), "derived");
    ASSERT_EQ(const_base_reference->get_static_name(), "base");
    ASSERT_FALSE(global_panic_handler.is_panic_active());
}

TEST_F(counted_casting_test, upcasting_construction_from_ref)
{
    saam::var<derived> derived_instance;

    saam::ref<derived> derived_reference = derived_instance;
    saam::ref<base> base_reference = derived_reference;
    ASSERT_EQ(derived_reference->get_dynamice_name(), "derived");
    ASSERT_EQ(derived_reference->get_static_name(), "derived");
    ASSERT_EQ(base_reference->get_dynamice_name(), "derived");
    ASSERT_EQ(base_reference->get_static_name(), "base");
    ASSERT_FALSE(global_panic_handler.is_panic_active());

    saam::ref<const derived> derived_const_reference = derived_instance;
    saam::ref<const base> base_const_reference = derived_const_reference;
    ASSERT_EQ(derived_const_reference->get_dynamice_name(), "derived");
    ASSERT_EQ(derived_const_reference->get_static_name(), "derived");
    ASSERT_EQ(base_const_reference->get_dynamice_name(), "derived");
    ASSERT_EQ(base_const_reference->get_static_name(), "base");
    ASSERT_FALSE(global_panic_handler.is_panic_active());
}

TEST_F(counted_casting_test, upcasting_assignment_from_var)
{
    saam::var<derived> derived_instance;

    saam::ref<base> base_reference = derived_instance;
    ASSERT_EQ(base_reference->get_dynamice_name(), "derived");
    ASSERT_EQ(base_reference->get_static_name(), "base");
    ASSERT_FALSE(global_panic_handler.is_panic_active());

    saam::ref<const base> base_const_reference = derived_instance;
    ASSERT_EQ(base_const_reference->get_dynamice_name(), "derived");
    ASSERT_EQ(base_const_reference->get_static_name(), "base");
    ASSERT_FALSE(global_panic_handler.is_panic_active());
}

TEST_F(counted_casting_test, upcasting_assignment_from_ref)
{
    saam::var<derived> derived_instance;

    saam::ref<derived> derived_reference = derived_instance;
    saam::ref<base> base_reference = derived_reference;
    ASSERT_EQ(derived_reference->get_dynamice_name(), "derived");
    ASSERT_EQ(derived_reference->get_static_name(), "derived");
    ASSERT_EQ(base_reference->get_dynamice_name(), "derived");
    ASSERT_EQ(base_reference->get_static_name(), "base");
    ASSERT_FALSE(global_panic_handler.is_panic_active());

    saam::ref<const derived> derived_const_reference = derived_instance;
    saam::ref<const base> base_const_reference = derived_const_reference;
    ASSERT_EQ(derived_const_reference->get_dynamice_name(), "derived");
    ASSERT_EQ(derived_const_reference->get_static_name(), "derived");
    ASSERT_EQ(base_const_reference->get_dynamice_name(), "derived");
    ASSERT_EQ(base_const_reference->get_static_name(), "base");
    ASSERT_FALSE(global_panic_handler.is_panic_active());
}

TEST_F(counted_casting_test, static_downcasting_from_ref)
{
    saam::var<derived> derived_instance;

    saam::ref<base> base_reference = derived_instance;
    saam::ref<derived> derived_reference = base_reference.static_down_cast<derived>();
    ASSERT_EQ(derived_reference->get_dynamice_name(), "derived");
    ASSERT_EQ(derived_reference->get_static_name(), "derived");
    ASSERT_EQ(base_reference->get_dynamice_name(), "derived");
    ASSERT_EQ(base_reference->get_static_name(), "base");
    ASSERT_FALSE(global_panic_handler.is_panic_active());

    saam::ref<const derived> derived_const_reference = base_reference.static_down_cast<const derived>();
    ASSERT_EQ(derived_const_reference->get_dynamice_name(), "derived");
    ASSERT_EQ(derived_const_reference->get_static_name(), "derived");
    ASSERT_FALSE(global_panic_handler.is_panic_active());

    saam::ref<const base> base_const_reference = derived_instance;
    saam::ref<const derived> derived_const_reference2 = base_const_reference.static_down_cast<const derived>();
    ASSERT_EQ(derived_const_reference2->get_dynamice_name(), "derived");
    ASSERT_EQ(derived_const_reference2->get_static_name(), "derived");
    ASSERT_EQ(base_const_reference->get_dynamice_name(), "derived");
    ASSERT_EQ(base_const_reference->get_static_name(), "base");
    ASSERT_FALSE(global_panic_handler.is_panic_active());
}

TEST_F(counted_casting_test, dynamic_downcasting_from_ref)
{
    saam::var<derived> derived_instance;

    saam::ref<base> base_reference = derived_instance;
    saam::ref<derived> derived_reference = base_reference.dynamic_down_cast<derived>();
    ASSERT_EQ(derived_reference->get_dynamice_name(), "derived");
    ASSERT_EQ(derived_reference->get_static_name(), "derived");
    ASSERT_EQ(base_reference->get_dynamice_name(), "derived");
    ASSERT_EQ(base_reference->get_static_name(), "base");
    ASSERT_FALSE(global_panic_handler.is_panic_active());

    saam::ref<const derived> derived_const_reference = base_reference.dynamic_down_cast<const derived>();
    ASSERT_EQ(derived_const_reference->get_dynamice_name(), "derived");
    ASSERT_EQ(derived_const_reference->get_static_name(), "derived");
    ASSERT_FALSE(global_panic_handler.is_panic_active());

    saam::ref<const base> base_const_reference = derived_instance;
    saam::ref<const derived> derived_const_reference2 = base_const_reference.dynamic_down_cast<const derived>();
    ASSERT_EQ(derived_const_reference2->get_dynamice_name(), "derived");
    ASSERT_EQ(derived_const_reference2->get_static_name(), "derived");
    ASSERT_EQ(base_const_reference->get_dynamice_name(), "derived");
    ASSERT_EQ(base_const_reference->get_static_name(), "base");
    ASSERT_FALSE(global_panic_handler.is_panic_active());

    EXPECT_THROW(base_const_reference.dynamic_down_cast<const derived2>(), std::bad_cast);
}

}  // namespace saam::test
