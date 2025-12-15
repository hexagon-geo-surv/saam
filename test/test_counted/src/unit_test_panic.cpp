// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/safe_ref.hpp>

#include <typeinfo>

namespace saam
{

void dangling_reference_panic(const std::type_info &var_type, void *var_instance, std::size_t num_dangling_references) noexcept
{
}

}  // namespace saam
