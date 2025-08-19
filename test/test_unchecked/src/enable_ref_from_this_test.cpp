// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#include <saam/enable_ref_from_this.hpp>
#include <saam/var.hpp>
#include <saam/ref.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <utility>

namespace saam::test
{

class unchecked_enable_ref_from_this_test : public ::testing::Test
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

TEST_F(unchecked_enable_ref_from_this_test, happy_flow)
{
    saam::var<my_class> my_instance(std::in_place);

    auto callback = my_instance.borrow()->generate_callback();

    ASSERT_EQ(int(6), callback(5));

    ASSERT_FALSE(global_panic_handler.is_panic_active());
}

}  // namespace saam::test
