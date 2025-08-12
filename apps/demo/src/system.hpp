// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include "component_a.hpp"
#include "component_b.hpp"

#include <saam/var.hpp>

#include <utility>

namespace demo
{

class system
{
  public:
    system()
        : component_b_(std::in_place, make_any_ptr(component_a_))
    {
        // Fine tune the stack tracing for each individual instances
        // component_a_.enable_instance_stack_tracking(true);
        // component_b_.enable_instance_stack_tracking(true);
    }

    void run()
    {
        auto component_b_ref = component_b_.borrow();
        component_b_ref->do_something();
    }

  private:
    // The system is the owner of the components, only the system may destroy the components
    saam::var<component_a> component_a_;
    saam::var<component_b> component_b_;
};

}  // namespace demo
