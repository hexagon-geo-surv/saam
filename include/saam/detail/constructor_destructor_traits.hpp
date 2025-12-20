// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/detail/borrow_manager_traits.hpp>    
#include <concepts>

namespace saam
{

template <typename T, typename TBorrowManager>
concept has_post_constructor = requires(T t, TBorrowManager borrow_mgr) {
    { t.post_constructor(borrow_mgr) } -> std::same_as<void>;
};

template <typename T>
concept has_pre_destructor = requires(T t) {
    { t.pre_destructor() } -> std::same_as<void>;
};

}  // namespace saam
