// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#include <saam/safe_ref.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <utility>

namespace saam::test
{

class unchecked_enable_ref_from_this_test : public ::testing::Test
{
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
}

}  // namespace saam::test
