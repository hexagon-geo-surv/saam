// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/safe_ref.hpp>

#include <stacktrace>
#include <typeinfo>

namespace saam
{

void dangling_reference_panic(const std::type_info &var_type,
                              void *var_instance,
                              const std::stacktrace &dangling_ref_creation_stack) noexcept
{
    int a = 5;
}

}  // namespace saam
