// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <concepts>
#include <type_traits>
#include <typeinfo>

namespace saam
{

template <typename TBorrowManager>
concept borrow_manager = requires(TBorrowManager instance, typename TBorrowManager::ref_base ref_base) {
    typename TBorrowManager::ref_base;

    { ref_base.borrow_manager() } -> std::convertible_to<TBorrowManager *>;
    { ref_base.is_managed() } -> std::convertible_to<bool>;
};

template <typename T>
concept forward_declared = !requires { sizeof(T); };

template <typename T>
concept nothrow_movable = std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T>;

// Forward declared typed are accepted as underlying type, because there is no way to check their properties.
// Const types are not movable, so no nothrow_movable is necessary
// For complete, non const types the the move and destruction shall be nothrow.
template <typename T>
concept underlying_type = forward_declared<T> || std::is_const_v<T> || nothrow_movable<T>;

template <typename TBorrowManager>
concept has_verify_dangling_references = requires(TBorrowManager instance) {
    instance.verify_dangling_references(typeid(int), nullptr);
};

template <typename TBorrowManager>
concept has_noexcept_verify_dangling_references = requires(TBorrowManager instance) {
    { instance.verify_dangling_references(typeid(int), nullptr) } noexcept;
};

}  // namespace saam
