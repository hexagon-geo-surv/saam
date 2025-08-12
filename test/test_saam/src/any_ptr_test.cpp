// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#include <saam/any_ptr.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory.h>
#include <string>
#include <utility>

namespace
{

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

}  // namespace

namespace saam::test
{

class any_ptr_test : public ::testing::Test
{
};

TEST_F(any_ptr_test, default_constructor)
{
    saam::any_ptr<base> base_ptr;
    ASSERT_FALSE(base_ptr);
}

TEST_F(any_ptr_test, construct_from_null_pointer)
{
    saam::any_ptr<base> base_ptr = {};
    ASSERT_FALSE(base_ptr);
}

TEST_F(any_ptr_test, create_from_raw_reference)
{
    derived derived_instance;
    saam::any_ptr<base> base_ptr = saam::make_any_ptr(derived_instance);

    ASSERT_EQ(base_ptr->get_static_name(), "base");
    ASSERT_EQ(base_ptr->get_dynamice_name(), "derived");
}

TEST_F(any_ptr_test, create_from_raw_pointer)
{
    derived derived_instance;
    saam::any_ptr<base> base_ptr = saam::make_any_ptr(&derived_instance);

    ASSERT_EQ(base_ptr->get_static_name(), "base");
    ASSERT_EQ(base_ptr->get_dynamice_name(), "derived");
}

TEST_F(any_ptr_test, create_from_shared_pointer)
{
    auto derived_instance = std::make_shared<derived>();
    saam::any_ptr<base> base_ptr = saam::make_any_ptr(derived_instance);

    ASSERT_EQ(base_ptr->get_static_name(), "base");
    ASSERT_EQ(base_ptr->get_dynamice_name(), "derived");
}

TEST_F(any_ptr_test, create_from_smart_variable)
{
    saam::var<derived> derived_instance;
    saam::any_ptr<base> base_ptr = saam::make_any_ptr(derived_instance);

    ASSERT_EQ(base_ptr->get_static_name(), "base");
    ASSERT_EQ(base_ptr->get_dynamice_name(), "derived");
}

TEST_F(any_ptr_test, create_from_smart_reference)
{
    saam::var<base> base_instance;
    auto base_ref = base_instance.borrow();
    saam::any_ptr<base> base_ptr = saam::make_any_ptr(base_ref);

    ASSERT_EQ(base_ptr->get_static_name(), "base");
    ASSERT_EQ(base_ptr->get_dynamice_name(), "base");
}

TEST_F(any_ptr_test, copy_construct)
{
    base base_instance;
    saam::any_ptr<base> base_ptr(saam::make_any_ptr(base_instance));
    saam::any_ptr<base> base_ptr2(base_ptr);

    ASSERT_EQ(base_ptr2->get_static_name(), "base");
    ASSERT_EQ(base_ptr2->get_dynamice_name(), "base");
}

TEST_F(any_ptr_test, move_construct)
{
    base base_instance;
    saam::any_ptr<base> base_ptr(saam::make_any_ptr(base_instance));
    saam::any_ptr<base> base_ptr2(std::move(base_ptr));

    ASSERT_EQ(base_ptr2->get_static_name(), "base");
    ASSERT_EQ(base_ptr2->get_dynamice_name(), "base");
}

TEST_F(any_ptr_test, construct_from_derived)
{
    derived derived_instance;
    saam::any_ptr<base> base_ptr(saam::make_any_ptr(derived_instance));

    ASSERT_EQ(base_ptr->get_static_name(), "base");
    ASSERT_EQ(base_ptr->get_dynamice_name(), "derived");
}

TEST_F(any_ptr_test, copy_assignment)
{
    base base_instance;
    saam::any_ptr<base> base_ptr(saam::make_any_ptr(base_instance));
    saam::any_ptr<base> base_ptr2;
    base_ptr2 = base_ptr;

    ASSERT_EQ(base_ptr2->get_static_name(), "base");
    ASSERT_EQ(base_ptr2->get_dynamice_name(), "base");
}

TEST_F(any_ptr_test, move_assignment)
{
    base base_instance;
    saam::any_ptr<base> base_ptr(saam::make_any_ptr(base_instance));
    saam::any_ptr<base> base_ptr2;
    base_ptr2 = std::move(base_ptr);

    ASSERT_EQ(base_ptr2->get_static_name(), "base");
    ASSERT_EQ(base_ptr2->get_dynamice_name(), "base");
}

TEST_F(any_ptr_test, reset_and_empty)
{
    base base_instance;
    saam::any_ptr<base> base_ptr = saam::make_any_ptr(base_instance);
    base_ptr.reset();
    ASSERT_FALSE(base_ptr);
}

TEST_F(any_ptr_test, dereference_operator)
{
    base base_instance;
    saam::any_ptr<base> base_ptr = saam::make_any_ptr(base_instance);

    ASSERT_EQ((*base_ptr).get_static_name(), "base");
}

TEST_F(any_ptr_test, arrow_operator)
{
    derived derived_instance;
    saam::any_ptr<base> base_ptr = saam::make_any_ptr(derived_instance);

    ASSERT_EQ(base_ptr->get_dynamice_name(), "derived");
}

TEST_F(any_ptr_test, const_correctness)
{
    const derived derived_instance;
    saam::any_ptr<const base> base_ptr = saam::make_any_ptr(derived_instance);

    ASSERT_EQ(base_ptr->get_dynamice_name(), "derived");
}
}  // namespace saam::test
