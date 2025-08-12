// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#include <saam/panic.hpp>
#include <saam/var.hpp>

#include "system.hpp"

#include <iostream>

int main()
{
    saam::global_panic_handler.set_panic_action([](std::string_view errmsg) {
        std::cerr << "Panic: " << errmsg << std::endl << std::flush;
        std::abort();
    });

    // Set the stack tracing for all demo::component_b instances - only works in tracked mode
    // saam::var<demo::component_b>::enable_type_stack_tracking(true);

    demo::system demo_system;
    demo_system.run();

    return 0;
}
