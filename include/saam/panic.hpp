// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <functional>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>

namespace saam
{

class panic_handler
{
  public:
    void set_panic_action(std::function<void(std::string_view)> panic_action)
    {
        panic_action_ = std::move(panic_action);
    }

    void trigger_panic(std::string_view errmsg)
    {
        panic_triggered_ = true;
        panic_message_ = errmsg;
        if (panic_action_)
        {
            panic_action_(panic_message_);
        }
    }

    bool is_panic_active() const
    {
        return panic_triggered_;
    }

    std::string_view panic_message() const
    {
        return panic_message_;
    }

    void clear_panic()
    {
        panic_triggered_ = false;
        panic_message_.clear();
    }

  private:
    bool panic_triggered_ = false;
    std::string panic_message_;
    std::function<void(std::string_view)> panic_action_ = [](std::string_view errmsg) {
        std::cerr << "Panic: " << errmsg << '\n' << std::flush;
        std::abort();
    };
};

inline panic_handler global_panic_handler;

inline void assert_that(bool predicate, std::string_view errmsg)
{
    if (!predicate)
    {
        global_panic_handler.trigger_panic(errmsg);
    }
}

}  // namespace saam
