// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/detail/basic_ref.hpp>
#include <saam/detail/borrow_manager.hpp>

#include <utility>

namespace saam
{

template <class T, borrow_manager TBorrowManager>
class basic_enable_ref_from_this;

template <class T, borrow_manager TBorrowManager>
class basic_var
{
  public:
    basic_var();

    template <typename... Args>
    explicit basic_var(std::in_place_t, Args &&...args);

    explicit basic_var(const T &instance);

    explicit basic_var(T &&instance);

    // No conversion copy constructor, because of the slicing of T - only the base class would be copied
    basic_var(const basic_var &other);

    // No conversion move constructor, because of the slicing of T - only the base class would be copied
    basic_var(basic_var &&other) noexcept;

    // No conversion copy assignment, because of the slicing of T - only the base class would be copied
    basic_var &operator=(const basic_var &other) noexcept;

    // No conversion move assignment, because of the slicing of T - only the base class would be copied
    basic_var &operator=(basic_var &&other) noexcept;

    ~basic_var();

    // Borrowing is a const operation, because borrowing just provides access to the underlying object - does not change the manager
    [[nodiscard]] basic_ref<T, TBorrowManager> borrow() const noexcept;

    // No direct casting to raw reference is allowed
    [[nodiscard]] operator T &() const = delete;

  private:
    void call_pre_destructor();

    void configure_enable_ref_from_this();

    void call_post_constructor();

    template <typename TOther, borrow_manager TOtherBorrowManager>
    friend class basic_var;

    template <typename TOther, borrow_manager TOtherBorrowManager>
    friend class basic_ref;

    // Keeping the instance at the first position to have the same address for the var and the instance
    T instance_;
    mutable TBorrowManager borrow_manager_;
};

}  // namespace saam
