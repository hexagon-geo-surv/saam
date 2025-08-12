// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/modes.hpp>

#if REFCELL_BORROW_CHECKING_MODE == COUNTED
#include <saam/detail/counted_ref/var.hpp>
#elif REFCELL_BORROW_CHECKING_MODE == TRACKED
#include <saam/detail/tracked_ref/var.hpp>
#elif REFCELL_BORROW_CHECKING_MODE == UNCHECKED
#include <saam/detail/unchecked_ref/var.hpp>
#endif

