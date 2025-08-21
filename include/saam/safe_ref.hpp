// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/detail/basic_enable_ref_from_this.hpp>
#include <saam/detail/basic_ref.ipp>
#include <saam/detail/basic_var.ipp>
#include <saam/modes.hpp>

namespace saam
{

template <typename T>
using var = basic_var<T, current_borrow_manager_t>;

template <typename T>
using ref = basic_ref<T, current_borrow_manager_t>;

template <typename T>
using enable_ref_from_this = basic_enable_ref_from_this<T, current_borrow_manager_t>;

};  // namespace saam
