// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/safe_ref.hpp>

#include <sstream>
#include <string>
#include <string_view>
#include <typeinfo>

namespace saam
{

class unit_test_panic_handler
{
  public:
    void trigger_panic(std::string_view errmsg)
    {
        panic_triggered_ = true;
        panic_message_ = errmsg;
    }

    [[nodiscard]] bool is_panic_active() const
    {
        return panic_triggered_;
    }

    [[nodiscard]] std::string_view panic_message() const
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
};

inline unit_test_panic_handler test_panic_handler;

inline void dangling_reference_panic(const std::type_info &var_type, void *var_instance, std::size_t dangling_references) noexcept
{
    std::ostringstream panic_message;
    panic_message << "Borrow checked variable of type <" << var_type.name() << "> destroyed with " << dangling_references
                  << " active reference(s).\n";
    test_panic_handler.trigger_panic(panic_message.str());
}

}  // namespace saam
