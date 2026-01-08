// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/detail/basic_var.hpp>
#include <saam/detail/constructor_destructor_traits.hpp>
#include <saam/detail/unchecked_borrow_manager.hpp>

#include <type_traits>
#include <utility>

namespace saam
{

template <typename T, borrow_manager TBorrowManager>
basic_var<T, TBorrowManager>::basic_var()
{
    call_post_constructor();
}

template <typename T, borrow_manager TBorrowManager>
template <typename... Args>
basic_var<T, TBorrowManager>::basic_var(std::in_place_t, Args &&...args) :
    instance_(std::forward<Args>(args)...)
{
    call_post_constructor();
}

template <typename T, borrow_manager TBorrowManager>
basic_var<T, TBorrowManager>::basic_var(const T &instance) :
    instance_(instance)
{
    call_post_constructor();
}

template <typename T, borrow_manager TBorrowManager>
basic_var<T, TBorrowManager>::basic_var(T &&instance) :
    instance_(std::move(instance))
{
    call_post_constructor();
}

template <typename T, borrow_manager TBorrowManager>
basic_var<T, TBorrowManager>::basic_var(const basic_var &other) :
    // Always use the saam::var through a reference, so that the reference management can ensure existence of the underlying object
    // Foward the call to the underlying type's copy constructor
    basic_var(*other.borrow())
{
    // The post_constructor needs to be called, because the smart self reference has changed for the object
    call_post_constructor();
}

template <typename T, borrow_manager TBorrowManager>
basic_var<T, TBorrowManager>::basic_var(const basic_ref<T, TBorrowManager> &other) :
    // The other instance is guaranteed to be valid, because if is a reference
    // Foward the call to the underlying type's copy constructor
    basic_var(*other)
{
    // The post_constructor needs to be called, because the smart self reference has changed for the object
    call_post_constructor();
}

template <typename T, borrow_manager TBorrowManager>
basic_var<T, TBorrowManager>::basic_var(basic_var &&other) noexcept :
    // Always use the saam::var through a reference, so that the reference management can ensure existence of the underlying object
    // Foward the call to the underlying type's move constructor
    basic_var(std::move(*other.borrow()))
{
    // The post_constructor needs to be called, because the smart self reference has changed for the object
    call_post_constructor();
}

template <typename T, borrow_manager TBorrowManager>
basic_var<T, TBorrowManager> &basic_var<T, TBorrowManager>::operator=(const basic_var &other) noexcept
{
    // During the assignment, the other instance must be borrowed to ensure proper reference management
    return operator=(other.borrow());
}

template <typename T, borrow_manager TBorrowManager>
basic_var<T, TBorrowManager> &basic_var<T, TBorrowManager>::operator=(const basic_ref<T, TBorrowManager> &other) noexcept
{
    if (&instance_ == other.instance_)
    {
        return *this;
    }

    // Only the instance shall be assigned, reference management of "this" and "other" are independent
    instance_ = *other.instance_;
    return *this;
}

template <typename T, borrow_manager TBorrowManager>
basic_var<T, TBorrowManager> &basic_var<T, TBorrowManager>::operator=(basic_var &&other) noexcept
{
    basic_ref<T, TBorrowManager> other_borrow = other.borrow();
    if (&instance_ == other_borrow.instance_)
    {
        return *this;
    }

    // Only the instance shall be assigned, reference management of "this" and "other" are independent
    instance_ = std::move(*other_borrow.instance_);
    return *this;
}

template <typename T, borrow_manager TBorrowManager>
basic_var<T, TBorrowManager>::~basic_var()
{
    // Pre-destructor offers the possibility to do cleanup before the owned object is destroyed.
    // It is a great place to revoke callbacks that contain self references.
    if constexpr (has_pre_destructor<T>)
    {
        instance_.pre_destructor();
    }

    // Before destroying the owned object, we need to check if there are any active references
    // The desctruction of the owned object cannot be done before the reference check,
    // because it would allow a data race between a thread inside the owned object and the destructor of the owned object.
    if constexpr (can_check_dangling_references<TBorrowManager>)
    {
        borrow_manager_.verify_dangling_references(typeid(T), this);
    }
}

template <typename T, borrow_manager TBorrowManager>
template <typename... Args>
basic_var<T, TBorrowManager> &basic_var<T, TBorrowManager>::emplace(Args &&...args)
{
    // Destroy the current instance
    if constexpr (has_pre_destructor<T>)
    {
        instance_.pre_destructor();
    }
    instance_.~T();

    // No reference check needed here, because existing refrerences still point here and we maintain here a valid instance.

    // Construct a new one in place
    new (&instance_) T(std::forward<Args>(args)...);
    call_post_constructor();

    return *this;
}

template <typename T, borrow_manager TBorrowManager>
basic_ref<T, TBorrowManager> basic_var<T, TBorrowManager>::borrow() const noexcept
{
    T &instance = const_cast<basic_var *>(this)->instance_;
    if constexpr (std::is_same_v<TBorrowManager, unchecked_borrow_manager>)
    {
        return {instance, nullptr};
    }
    else
    {
        return {instance, &borrow_manager_};
    }
}

template <typename T, borrow_manager TBorrowManager>
basic_var<T, TBorrowManager>::operator basic_ref<T, TBorrowManager>() const noexcept
{
    return borrow();
}

template <typename T, borrow_manager TBorrowManager>
basic_ref<T, TBorrowManager> basic_var<T, TBorrowManager>::operator->() const noexcept
{
    return borrow();
}

template <typename T, borrow_manager TBorrowManager>
basic_var<T, TBorrowManager> &basic_var<T, TBorrowManager>::operator=(const T &instance) noexcept
{
    instance_ = instance;
    return *this;
}

template <typename T, borrow_manager TBorrowManager>
basic_var<T, TBorrowManager> &basic_var<T, TBorrowManager>::operator=(T &&instance) noexcept
{
    instance_ = std::move(instance);
    return *this;
}

template <typename T, borrow_manager TBorrowManager>
[[nodiscard]] bool basic_var<T, TBorrowManager>::operator==(const T &other) const noexcept
{
    // The borrow lifetime starts before the comparison and ends after it, so the comparison is made with a stable smart reference
    return *borrow() == (other);
}

template <typename T, borrow_manager TBorrowManager>
[[nodiscard]] bool basic_var<T, TBorrowManager>::operator!=(const T &other) const noexcept
{
    // The borrow lifetime starts before the comparison and ends after it, so the comparison is made with a stable smart reference
    return *borrow() != (other);
}

template <typename T, borrow_manager TBorrowManager>
void basic_var<T, TBorrowManager>::call_post_constructor()
{
    if constexpr (has_post_constructor<T, TBorrowManager>)
    {
        if constexpr (std::is_same_v<TBorrowManager, unchecked_borrow_manager>)
        {
            instance_.post_constructor(basic_ref<T, TBorrowManager>{instance_, nullptr});
        }
        else
        {
            instance_.post_constructor(basic_ref<T, TBorrowManager>{instance_, &borrow_manager_});
        }
    }
}

}  // namespace saam
