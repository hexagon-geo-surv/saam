// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/detail/borrow_manager.hpp>
#include <saam/panic.hpp>

#include <type_traits>

namespace saam
{

template <typename T, borrow_manager TBorrowManager>
class basic_var;

template <typename T, borrow_manager TBorrowManager>
class basic_ref : private TBorrowManager::ref_base
{
  public:
    // Unmanaged reference constructor
    // Used with legacy client, which only has raw references
    // This case, there is no borrow checking
    explicit basic_ref(T &instance)
        : instance_(&instance)
    {
    }

    // Conversion copy constructor
    template <typename TOther>
        requires std::is_convertible_v<TOther *, T *>
    basic_ref(const basic_ref<TOther, TBorrowManager> &other)
        : TBorrowManager::ref_base(other)
        , instance_(other.instance_)
    {
    }

    // The conversion copy constructor does not cover the copy constructor, so we need to implement it explicitly
    basic_ref(const basic_ref &other)
        : TBorrowManager::ref_base(other)
        , instance_(other.instance_)
    {
    }

    // The requirement for moved-from objects is specified in section [lib.types.movedfrom] of the C++20 standard (section 16.5.5.15), which
    // states: "Unless otherwise specified, such moved-from objects shall be placed in a valid but unspecified state."
    // The key aspects of this requirement:
    // Valid state: The moved-from object must remain valid - it must be destructible and assignable
    // Unspecified state: No specific value guarantees beyond validity
    // Implementation flexibility: Library implementers can choose how to represent the moved-from state

    // Conversion move constructor
    template <typename TOther>
        requires std::is_convertible_v<TOther *, T *>
    basic_ref(basic_ref<TOther, TBorrowManager> &&other) noexcept
        : TBorrowManager::ref_base(std::move(other))
        , instance_(other.instance_)
    {
        other.instance_ = nullptr;
    }

    // The conversion move constructor does not cover the move constructor, so we need to implement it explicitly
    basic_ref(basic_ref &&other) noexcept
        : TBorrowManager::ref_base(std::move(other))
        , instance_(other.instance_)
    {
        other.instance_ = nullptr;
    }

    // Conversion copy construction from var
    template <typename TVar>
        requires std::is_convertible_v<TVar *, T *>
    basic_ref(const basic_var<TVar, TBorrowManager> &var) noexcept;

    // Conversion copy assignment operator from ref
    template <typename TOther>
        requires std::is_convertible_v<TOther *, T *>
    basic_ref &operator=(const basic_ref<TOther, TBorrowManager> &other)
    {
        instance_ = other.instance_;

        TBorrowManager::ref_base::operator=(other);

        return *this;
    }

    basic_ref &operator=(const basic_ref &other)
    {
        if (this == &other)
        {
            return *this;
        }

        instance_ = other.instance_;

        TBorrowManager::ref_base::operator=(other);

        return *this;
    }

    // Conversion move assignment operator from ref
    template <typename TOther>
        requires std::is_convertible_v<TOther *, T *>
    basic_ref &operator=(basic_ref<TOther, TBorrowManager> &&other) noexcept
    {
        instance_ = other.instance_;
        other.instance_ = nullptr;

        TBorrowManager::ref_base::operator=(std::move(other));

        return *this;
    }

    basic_ref &operator=(basic_ref &&other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }

        instance_ = other.instance_;
        other.instance_ = nullptr;

        TBorrowManager::ref_base::operator=(std::move(other));

        return *this;
    }

    // Conversion assignment operator from var
    template <typename TOther>
        requires std::is_convertible_v<TOther *, T *>
    basic_ref &operator=(const basic_var<TOther, TBorrowManager> &var) noexcept;

    ~basic_ref() = default;

    // Equality of references, not the underlying objects --> similar to smart pointers
    [[nodiscard]] bool operator==(const basic_ref &other) const noexcept
    {
        return instance_ == other.instance_;
    }

    [[nodiscard]] bool operator!=(const basic_ref &other) const noexcept
    {
        return !(instance_ == other.instance_);
    }

    // Arrow operator
    [[nodiscard]] T *operator->() const noexcept
    {
        // A reference is always bound to an object, so no check is needed - unless it is in a moved from state
        return instance_;
    }

    // Dereference operator
    [[nodiscard]] T &operator*() const noexcept
    {
        // A reference is always bound to an object, so no check is needed - unless it is in a moved from state
        return *instance_;
    }

    // Cast to T reference
    [[nodiscard]] explicit operator T &() const noexcept
    {
        // A reference is always bound to an object, so no check is needed - unless it is in a moved from state
        return *instance_;
    }

    // Cast to T pointer
    [[nodiscard]] explicit operator T *() const noexcept
    {
        // A reference is always bound to an object, so no check is needed - unless it is in a moved from state
        return instance_;
    }

    // Downcasting to a derived type - without type checking
    template <typename TOther>
        requires std::is_base_of_v<T, TOther>
    basic_ref<TOther, TBorrowManager> static_down_cast() const
    {
        // This is an explicit operation, so it is safe to cast
        return basic_ref<TOther, TBorrowManager>(static_cast<TOther &>(*instance_), *TBorrowManager::ref_base::borrow_manager());
    }

    // Downcasting to a derived type - without RTTI type checking
    // Throws std::bad_cast if the cast is not valid
    template <typename TOther>
        requires std::is_base_of_v<T, TOther>
    basic_ref<TOther, TBorrowManager> dynamic_down_cast() const
    {
        // This is an explicit operation, so it is safe to cast
        return basic_ref<TOther, TBorrowManager>(dynamic_cast<TOther &>(*instance_), *TBorrowManager::ref_base::borrow_manager());
    }

  private:
    template <typename TOther, borrow_manager TOtherBorrowManager>
    friend class basic_var;

    template <typename TOther, borrow_manager TOtherBorrowManager>
    friend class basic_ref;

    basic_ref(T &instance, TBorrowManager &borrow_manager)
        : TBorrowManager::ref_base(borrow_manager)
        , instance_(&instance)
    {
    }

    T *instance_ = nullptr;
};

}  // namespace saam
