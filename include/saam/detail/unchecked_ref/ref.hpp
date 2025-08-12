// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/panic.hpp>

namespace saam
{

template <typename T>
class var;

template <typename T>
class ref
{
  public:
    // Unmanaged reference constructor
    // Used with legacy client, which only has raw references
    // This case, there is no borrow checking
    explicit ref(T &instance)
        : instance_(&instance)
    {
    }

    // Conversion copy constructor
    template <typename TOther>
        requires std::is_convertible_v<TOther *, T *>
    ref(const ref<TOther> &other)
        : instance_(other.instance_)
    {
    }

    // The conversion copy constructor does not cover the copy constructor, so we need to implement it explicitly
    ref(const ref &other)
        : instance_(other.instance_)
    {
    }

    // Conversion copy construction from var
    template <typename TVar>
        requires std::is_convertible_v<TVar *, T *>
    ref(const var<TVar> &var) noexcept;

    // Conversion copy assignment operator from ref
    template <typename TOther>
        requires std::is_convertible_v<TOther *, T *>
    ref &operator=(const ref<TOther> &other)
    {
        instance_ = other.instance_;

        return *this;
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
    ref(ref<TOther> &&other) noexcept
        : instance_(other.instance_)
    {
        other.instance_ = nullptr;
    }

    // The conversion move constructor does not cover the move constructor, so we need to implement it explicitly
    ref(ref &&other) noexcept
        : instance_(other.instance_)
    {
        other.instance_ = nullptr;
    }

    ref &operator=(const ref &other)
    {
        instance_ = other.instance_;

        return *this;
    }

    // Conversion move assignment operator from ref
    template <typename TOther>
        requires std::is_convertible_v<TOther *, T *>
    ref &operator=(ref<TOther> &&other) noexcept
    {
        instance_ = other.instance_;
        other.instance_ = nullptr;

        return *this;
    }

    ref &operator=(ref &&other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }
        instance_ = other.instance_;
        other.instance_ = nullptr;

        return *this;
    }

    // Conversion assignment operator from var
    template <typename TVar>
        requires std::is_convertible_v<TVar *, T *>
    ref &operator=(const var<TVar> &var) noexcept;

    ~ref() = default;

    // Equality of references, not the underlying objects --> similar to smart pointers
    [[nodiscard]] bool operator==(const ref &other) const noexcept
    {
        return instance_ == other.instance_;
    }

    [[nodiscard]] bool operator!=(const ref &other) const noexcept
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

    // If the underlying raw pointer is managed reference
    [[nodiscard]] bool is_managed() const
    {
        return false;
    }

    // Downcasting to a derived type - without type checking
    template <typename TOther>
        requires std::is_base_of_v<T, TOther>
    ref<TOther> static_down_cast() const
    {
        // This is an explicit operation, so it is safe to cast
        return ref<TOther>(static_cast<TOther&>(*instance_));
    }

    // Downcasting to a derived type - without RTTI type checking
    // Throws std::bad_cast if the cast is not valid
    template <typename TOther>
        requires std::is_base_of_v<T, TOther>
    ref<TOther> dynamic_down_cast() const
    {
        // This is an explicit operation, so it is safe to cast
        return ref<TOther>(dynamic_cast<TOther&>(*instance_));
    }

  private:
    template <typename TVar>
    friend class var;

    template <typename TRef>
    friend class ref;

    T *instance_ = nullptr;
};

}  // namespace saam
