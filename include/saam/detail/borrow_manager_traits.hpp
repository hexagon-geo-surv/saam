// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <concepts>
#include <typeinfo>

namespace saam
{

template <typename TBorrowManager>
concept borrow_manager = requires(TBorrowManager instance, typename TBorrowManager::ref_base ref_base) {
    typename TBorrowManager::ref_base;

    { ref_base.borrow_manager() };
    { ref_base.is_managed() } -> std::convertible_to<bool>;
};

template <typename T, typename TBorrowManager>
concept can_check_dangling_references = requires(TBorrowManager instance) {
    { instance.verify_dangling_references(typeid(T), static_cast<T *>(nullptr)) } -> std::same_as<void>;
};

}  // namespace saam
