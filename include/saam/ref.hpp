// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/detail/borrow_manager_traits.hpp>
#include <saam/detail/default_borrow_manager.hpp>

#include <type_traits>

namespace saam
{

template <typename T, borrow_manager TBorrowManager>
class var;

template <typename T, borrow_manager TBorrowManager = default_borrow_manager_t>
class ref : private TBorrowManager::ref_base
{
  public:
    using type_t = T;
    using borrow_manager_t = TBorrowManager;

    // Unmanaged reference constructor
    ref(T &instance);

    // Conversion copy constructor
    template <typename TOther>
        requires std::is_convertible_v<TOther *, T *>
    ref(const ref<TOther, TBorrowManager> &other);

    // The conversion copy constructor does not cover the copy constructor, so we need to implement it explicitly
    ref(const ref &other);

    // The requirement for moved-from objects is specified in section [lib.types.movedfrom] of the C++20 standard (section 16.5.5.15), which
    // states: "Unless otherwise specified, such moved-from objects shall be placed in a valid but unspecified state."
    // The key aspects of this requirement:
    // Valid state: The moved-from object must remain valid - it must be destructible and assignable
    // Unspecified state: No specific value guarantees beyond validity
    // Implementation flexibility: Library implementers can choose how to represent the moved-from state

    // Conversion move constructor
    template <typename TOther>
        requires std::is_convertible_v<TOther *, T *>
    ref(ref<TOther, TBorrowManager> &&other) noexcept;

    // The conversion move constructor does not cover the move constructor, so we need to implement it explicitly
    ref(ref &&other) noexcept;

    // Conversion copy construction from var
    template <typename TOther>
        requires std::is_convertible_v<TOther *, T *>
    ref(const var<TOther, TBorrowManager> &var) noexcept;

    // Conversion copy assignment operator from ref
    template <typename TOther>
        requires std::is_convertible_v<TOther *, T *>
    ref &operator=(const ref<TOther, TBorrowManager> &other);

    ref &operator=(const ref &other);

    // Conversion move assignment operator from ref
    template <typename TOther>
        requires std::is_convertible_v<TOther *, T *>
    ref &operator=(ref<TOther, TBorrowManager> &&other) noexcept;

    ref &operator=(ref &&other) noexcept;

    // Conversion assignment operator from var
    template <typename TOther>
        requires std::is_convertible_v<TOther *, T *>
    ref &operator=(const var<TOther, TBorrowManager> &other) noexcept;

    ~ref() = default;

    // Equality of references, not the underlying objects --> similar to smart pointers
    [[nodiscard]] bool operator==(const ref &other) const noexcept;

    [[nodiscard]] bool operator!=(const ref &other) const noexcept;

    // Arrow operator
    [[nodiscard]] T *operator->() const noexcept;

    // Dereference operator
    [[nodiscard]] T &operator*() const noexcept;

    // Cast to T reference
    [[nodiscard]] explicit operator T &() const noexcept;

    // Cast to T pointer
    [[nodiscard]] explicit operator T *() const noexcept;

    // Downcasting to a derived type - without type checking
    template <typename TOther>
        requires std::is_base_of_v<T, TOther>
    ref<TOther, TBorrowManager> static_down_cast() const;

    // Downcasting to a derived type - with RTTI type checking
    // Throws std::bad_cast if the cast is not valid
    template <typename TOther>
        requires std::is_base_of_v<T, TOther>
    ref<TOther, TBorrowManager> dynamic_down_cast() const;

  private:
    // Managed reference constructor
    ref(T &instance, TBorrowManager *borrow_manager);

    template <typename TOther, borrow_manager TOtherBorrowManager>
    friend class var;

    template <typename TOther, borrow_manager TOtherBorrowManager>
    friend class ref;

    T *instance_ = nullptr;
};

// Deduction guide
template <typename T, borrow_manager TBorrowManager>
ref(var<T, TBorrowManager>) -> ref<T, TBorrowManager>;

template <typename T, borrow_manager TBorrowManager>
ref(ref<T, TBorrowManager>) -> ref<T, TBorrowManager>;

}  // namespace saam

#include <saam/detail/ref.ipp>