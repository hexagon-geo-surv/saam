// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include "component_a.hpp"

#include <saam/any_ptr.hpp>
#include <saam/enable_ref_from_this.hpp>
#include <saam/ref.hpp>
#include <saam/var.hpp>

#include <functional>
#include <iostream>
#include <utility>

namespace demo
{

class component_b : public saam::enable_ref_from_this<component_b>
{
  public:
    explicit component_b(saam::any_ptr<component_a> comp_a)
        : comp_a_(std::move(comp_a))
    {
    }

    component_b(component_b &&other) noexcept = default;
    component_b &operator=(component_b &&other) noexcept = default;

    void post_constructor()
    {
        if (comp_a_)
        {
            // at this point the safe self reference is available
            comp_a_->register_callback([self = borrow_from_this()]() { std::cout << "component_b callback called\n"; });
        }
    }

    void do_something() const
    {
        if (comp_a_)
        {
            comp_a_->do_something();
        }
        std::cout << "component_b::do_something()\n";
    }

    void pre_destructor()
    {
        if (comp_a_)
        {
            // release the callbacks, so that we get rid of the self references in there
            comp_a_->register_callback({});
        }
    }

  private:
    // Component B just has a reference to component A,
    // it may use it, but cannot destroy it.
    saam::any_ptr<component_a> comp_a_;
};
}  // namespace demo
