// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#include <saam/safe_ref.hpp>

#include <typeinfo>

namespace saam
{

void dangling_reference_panic(const std::type_info &var_type, void *var_instance, std::size_t dangling_references) noexcept
{
    // We cannot return from this function, as the process state is corrupted due to dangling references.

    // Crash the test
    abort();
}

}  // namespace saam
