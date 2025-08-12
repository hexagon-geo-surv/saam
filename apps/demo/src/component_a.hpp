// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <functional>
#include <iostream>

namespace demo
{
class component_a
{
  public:
    component_a() = default;

    void register_callback(std::function<void()> callback)
    {
        callback_ = std::move(callback);
    }

    void do_something() const
    {
        std::cout << "component_a::do_something()" << std::endl;
        if (callback_)
        {
            callback_();
        }
    }

  private:
    std::function<void()> callback_;
};
}  // namespace demo
