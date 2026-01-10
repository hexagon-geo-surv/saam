// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/detail/ref.hpp>
#include <saam/detail/borrow_manager_traits.hpp>
#include <saam/detail/default_borrow_manager.hpp>

#include <utility>

namespace saam
{

template <typename T, borrow_manager TBorrowManager = default_borrow_manager_t>
class var
{
  public:
    using type_t = T;
    using borrow_manager_t = TBorrowManager;

    var();

    // In-place construction of the underlying type
    template <typename... Args>
    explicit var(std::in_place_t, Args &&...args);

    // Construct from underlying type
    explicit var(const T &instance);
    explicit var(T &&instance);

    // No conversion copy constructor, because of the slicing of T - only the base class would be copied
    var(const var &other);
    template <borrow_manager TOtherBorrowManager>
    var(const var<T, TOtherBorrowManager> &other);

    var(const ref<T, TBorrowManager> &other);
    template <borrow_manager TOtherBorrowManager>
    var(const ref<T, TOtherBorrowManager> &other);

    // No conversion move constructor, because of the slicing of T - only the base class would be copied
    var(var<T, TBorrowManager> &&other) noexcept;
    template <borrow_manager TOtherBorrowManager>
    var(var<T, TOtherBorrowManager> &&other) noexcept;
    // Makes no sense, as we do not extract anything from a rvalue ref, so we can always use the copy version of this function
    // var(ref<T, TBorrowManager> &&other) noexcept;

    // No conversion copy assignment, because of the slicing of T - only the base class would be copied
    var &operator=(const var &other) noexcept;
    template <borrow_manager TOtherBorrowManager>
    var &operator=(const var<T, TOtherBorrowManager> &other) noexcept;

    var &operator=(const ref<T, TBorrowManager> &other) noexcept;
    template <borrow_manager TOtherBorrowManager>
    var &operator=(const ref<T, TOtherBorrowManager> &other) noexcept;

    // No conversion move assignment, because of the slicing of T - only the base class would be copied
    var &operator=(var &&other) noexcept;
    template <borrow_manager TOtherBorrowManager>
    var &operator=(var<T, TOtherBorrowManager> &&other) noexcept;
    // Makes no sense, as we do not extract anything from a rvalue ref, so we can always use the copy version of this function
    // var &operator=(ref<T, TBorrowManager> &&other) noexcept;

    ~var();

    // In-place re-construction of the underlying type
    template <typename... Args>
    var &emplace(Args &&...args);

    // Borrowing is a const operation, because borrowing just provides access to the underlying object - does not change the manager
    [[nodiscard]] ref<T, TBorrowManager> borrow() const noexcept;

    // During the access to the undelying object, there must be a temporary smart reference. The lifetime of the temporary smart reference
    // starts before the operator-> is called and ends well after the call is completed. Without this, we use the underlying object without
    // administrating it in the borrow manager and a parallel destruction of the var would NOT consider this access for the final reference
    // check. The first operator-> provides a temporary smart reference. Then the call into the underlying object is done via the smart
    // reference's operator->. The two operators-> are collapsed into one operator-> by the C++ compiler.
    [[nodiscard]] ref<T, TBorrowManager> operator->() const noexcept;

    // Assignment from underlying type
    var &operator=(const T &instance) noexcept;
    var &operator=(T &&instance) noexcept;

    // Compare with underlying type
    [[nodiscard]] bool operator==(const T &other) const noexcept;
    [[nodiscard]] bool operator!=(const T &other) const noexcept;

    // No direct casting to raw reference is allowed
    [[nodiscard]] operator T &() const = delete;

  private:
    void call_post_constructor();

    template <typename TOther, borrow_manager TOtherBorrowManager>
    friend class var;

    template <typename TOther, borrow_manager TOtherBorrowManager>
    friend class ref;

    T instance_;
    // if TBorrowManager is unchecked_borrow_manager, this member is optimized away
#ifdef _MSC_VER
    [[msvc::no_unique_address]]
#else
    [[no_unique_address]]
#endif
    mutable TBorrowManager borrow_manager_;
};

}  // namespace saam
