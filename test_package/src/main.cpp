// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#include <saam/safe_ref.hpp>
#include <saam/version.hpp>

#include <iostream>

namespace saam
{
void dangling_reference_panic(const std::type_info &var_type, void *var_instance, std::size_t num_dangling_references) noexcept
{
    std::cerr << "Panic: saam::var of type <" << var_type.name() << "> at " << var_instance << " destroyed with " << num_dangling_references
              << " dangling references\n"
              << std::flush;
}
}  // namespace saam

int main()
{
    using namespace saam;
    std::cout << "saam version:" << version::major() << '.' << version::minor() << '.' << version::patch() << std::endl;

    saam::var<int> smart_variable{42};
    *smart_variable.borrow() = 22;
    std::cout << *smart_variable.borrow() << std::endl;

    return 0;
}
