// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/detail/borrow_manager_traits.hpp>
#include <saam/detail/default_borrow_manager.hpp>

#include <utility>

namespace saam
{

template <underlying_type T, borrow_manager TBorrowManager>
class ref;

template <underlying_type T, borrow_manager TBorrowManager = default_borrow_manager_t>
class var
{
  public:
    using type_t = T;
    using borrow_manager_t = TBorrowManager;

    //
    // From underlying type
    //

    var();
    // In-place construction of the underlying type
    template <typename... Args>
    explicit var(std::in_place_t, Args &&...args);
    explicit var(const T &instance);
    explicit var(T &&instance) noexcept;
    var &operator=(const T &instance);
    var &operator=(T &&instance) noexcept;

    //
    // From another var
    //

    // No conversion copy constructor, because of the slicing of T - only the base class would be copied
    var(const var &other);
    template <borrow_manager TOtherBorrowManager>
    var(const var<T, TOtherBorrowManager> &other);
    // Moving a var leaves the source var in a moved-from state. References to the target are still valid, but they refer to a moved-from
    // object. No conversion move constructor, because of the slicing of T - only the base class would be copied
    var(var<T, TBorrowManager> &&other) noexcept;
    template <borrow_manager TOtherBorrowManager>
    var(var<T, TOtherBorrowManager> &&other) noexcept;

    // No conversion copy assignment, because of the slicing of T - only the base class would be copied
    var &operator=(const var &other);
    template <borrow_manager TOtherBorrowManager>
    var &operator=(const var<T, TOtherBorrowManager> &other);

    // No conversion move assignment, because of the slicing of T - only the base class would be copied
    var &operator=(var &&other) noexcept;
    template <borrow_manager TOtherBorrowManager>
    var &operator=(var<T, TOtherBorrowManager> &&other) noexcept;

    //
    // From another ref
    //

    var(const ref<T, TBorrowManager> &other);
    template <borrow_manager TOtherBorrowManager>
    var(const ref<T, TOtherBorrowManager> &other);

    // Makes no sense, as we do not extract anything from a rvalue ref, so we can always use the copy version of this function
    // var(ref<T, TBorrowManager> &&other) ;

    var &operator=(const ref<T, TBorrowManager> &other);
    template <borrow_manager TOtherBorrowManager>
    var &operator=(const ref<T, TOtherBorrowManager> &other);

    // Makes no sense, as we do not extract anything from a rvalue ref, so we can always use the copy version of this function
    // var &operator=(ref<T, TBorrowManager> &&other) ;

    ~var();

    // No emplacement: if the constructor of the underlying type would throw, then the previous instance is already destroyed
    // and the new one could not be constructed, leaving the var in a empty raw-memory state:
    // underlying object is neither valid, nor in a moved-from state, but var has only uninitialized memory.
    // Use move assignment of the underlying type instead.
    template <typename... Args>
    var &emplace(Args &&...args) = delete;

    // Borrowing is a const operation, because borrowing just provides access to the underlying object - does not change the manager
    [[nodiscard]] ref<T, TBorrowManager> borrow() const noexcept;

    // During the access to the undelying object, there must be a temporary smart reference. The lifetime of the temporary smart reference
    // starts before the operator-> is called and ends well after the call is completed. Without this, we use the underlying object without
    // administrating it in the borrow manager and a parallel destruction of the var would NOT consider this access for the final reference
    // check. The first operator-> provides a temporary smart reference. Then the call into the underlying object is done via the smart
    // reference's operator->. The two operators-> are collapsed into one operator-> by the C++ compiler.
    [[nodiscard]] ref<T, TBorrowManager> operator->() const noexcept;

    // Compare with underlying type
    [[nodiscard]] bool operator==(const T &other) const noexcept;
    [[nodiscard]] bool operator!=(const T &other) const noexcept;

    // No direct casting to raw reference is allowed
    [[nodiscard]] operator T &() const = delete;

  private:
    void call_post_constructor() noexcept;
    void call_post_assignment() noexcept;

    template <underlying_type TOther, borrow_manager TOtherBorrowManager>
    friend class var;

    template <underlying_type TOther, borrow_manager TOtherBorrowManager>
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

#include <saam/detail/var.ipp>
