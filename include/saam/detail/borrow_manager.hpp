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

    { instance.verify_dangling_references(typeid(int), nullptr) };
    { ref_base.borrow_manager() };
    { ref_base.is_managed() } -> std::convertible_to<bool>;
};

}  // namespace saam
