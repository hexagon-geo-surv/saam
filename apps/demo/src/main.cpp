// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#include "system.hpp"

#include <saam/safe_ref.hpp>

#include <iostream>
#include <ostream>

namespace saam
{

#if SAAM_BORROW_CHECKING_MODE == 0
// Counted mode panic
void dangling_reference_panic(const std::type_info &var_type, void *var_instance, std::size_t num_dangling_references) noexcept
{
    std::cerr << "Panic: saam::var of type <" << var_type.name() << "> at " << var_instance << " destroyed with " << num_dangling_references
              << " dangling references\n"
              << std::flush;
}
#elif SAAM_BORROW_CHECKING_MODE == 1
// Tracked mode panic
void dangling_reference_panic(const std::type_info &var_type,
                              void *var_instance,
                              const std::stacktrace &var_destruction_stack,
                              std::size_t dangling_ref_index,
                              const std::stacktrace &dangling_ref_creation_stack) noexcept
{
    if (dangling_ref_index == 0)
    {
        std::cerr << "Panic: saam::var of type <" << var_type.name() << "> at " << var_instance
                  << " destroyed with dangling references. saam::var destroyed at:\n"
                  << var_destruction_stack << std::flush;
    }
    std::cerr << "\n---------- Dangling Reference #" << dangling_ref_index
              << " creation stack --------------------------------------------\n"
              << dangling_ref_creation_stack << '\n'
              << std::flush;
}
#endif

}  // namespace saam

int main()
{
    demo::system demo_system;
    demo_system.run();

    return 0;
}
