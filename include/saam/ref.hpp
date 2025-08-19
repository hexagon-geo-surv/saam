// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/modes.hpp>

#if REFCELL_BORROW_CHECKING_MODE == COUNTED

#include <saam/detail/counted_borrow_manager.hpp>
namespace saam
{
template <typename T>
using ref = basic_ref<T, counted_borrow_manager>;
}

#elif REFCELL_BORROW_CHECKING_MODE == TRACKED
#include <saam/detail/tracked_ref/ref.hpp>
#elif REFCELL_BORROW_CHECKING_MODE == UNCHECKED

#include <saam/detail/unchecked_borrow_manager.hpp>
namespace saam
{
template <typename T>
using ref = basic_ref<T, unchecked_borrow_manager>;
}

#endif
