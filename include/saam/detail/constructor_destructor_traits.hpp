// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/detail/borrow_manager_traits.hpp>

#include <concepts>
#include <type_traits>
#include <utility>

namespace saam
{

template <underlying_type T, borrow_manager TBorrowManager>
class ref;

template <typename T, typename TBorrowManager>
concept has_post_constructor = requires(T t, TBorrowManager borrow_mgr, saam::ref<T, TBorrowManager> t_ref) {
    { t.post_constructor(std::move(t_ref)) } -> std::same_as<void>;
};

template <typename T, typename TBorrowManager>
concept has_noexcept_post_constructor = requires(T t, saam::ref<T, TBorrowManager> t_ref) {
    { t.post_constructor(std::move(t_ref)) } noexcept -> std::same_as<void>;
};

template <typename T, typename TBorrowManager>
concept has_post_assignment = requires(T t, TBorrowManager borrow_mgr, saam::ref<T, TBorrowManager> t_ref) {
    { t.post_assignment(std::move(t_ref)) } -> std::same_as<void>;
};

template <typename T, typename TBorrowManager>
concept has_noexcept_post_assignment = requires(T t, saam::ref<T, TBorrowManager> t_ref) {
    { t.post_assignment(std::move(t_ref)) } noexcept -> std::same_as<void>;
};

template <typename T>
concept has_pre_destructor = requires(T t) {
    { t.pre_destructor() } -> std::same_as<void>;
};

template <typename T>
concept has_noexcept_pre_destructor = requires(T t) {
    { t.pre_destructor() } noexcept -> std::same_as<void>;
};

}  // namespace saam
