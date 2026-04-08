// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/detail/constructor_destructor_traits.hpp>
#include <saam/detail/unchecked_borrow_manager.hpp>

#include <memory>
#include <type_traits>
#include <utility>

namespace saam
{

template <underlying_type T, borrow_manager TBorrowManager>
var<T, TBorrowManager>::var()
{
    call_post_constructor();
}

template <underlying_type T, borrow_manager TBorrowManager>
template <typename... Args>
var<T, TBorrowManager>::var(std::in_place_t, Args &&...args) :
    instance_(std::forward<Args>(args)...)
{
    call_post_constructor();
}

template <underlying_type T, borrow_manager TBorrowManager>
var<T, TBorrowManager>::var(const T &instance) :
    instance_(instance)
{
    call_post_constructor();
}

template <underlying_type T, borrow_manager TBorrowManager>
var<T, TBorrowManager>::var(T &&instance) noexcept :
    instance_(std::move(instance))
{
    call_post_constructor();
}

template <underlying_type T, borrow_manager TBorrowManager>
var<T, TBorrowManager> &var<T, TBorrowManager>::operator=(const T &instance)
{
    if (std::addressof(instance_) == std::addressof(instance))
    {
        return *this;
    }

    instance_ = instance;
    call_post_assignment();

    return *this;
}

template <underlying_type T, borrow_manager TBorrowManager>
var<T, TBorrowManager> &var<T, TBorrowManager>::operator=(T &&instance) noexcept
{
    if (std::addressof(instance_) == std::addressof(instance))
    {
        return *this;
    }

    instance_ = std::move(instance);
    call_post_assignment();

    return *this;
}

template <underlying_type T, borrow_manager TBorrowManager>
var<T, TBorrowManager>::var(const var &other) :
    // Always use the saam::var through a reference, so that the reference management can ensure existence of the underlying object
    // Foward the call to the underlying type's copy constructor
    var(*other.borrow())
{
}

template <underlying_type T, borrow_manager TBorrowManager>
template <borrow_manager TOtherBorrowManager>
var<T, TBorrowManager>::var(const var<T, TOtherBorrowManager> &other) :
    // Always use the saam::var through a reference, so that the reference management can ensure existence of the underlying object
    // Foward the call to the underlying type's copy constructor
    var(*other.borrow())
{
}

template <underlying_type T, borrow_manager TBorrowManager>
var<T, TBorrowManager>::var(const ref<T, TBorrowManager> &other) :
    // The other instance is guaranteed to be valid, because if is a reference
    // Foward the call to the underlying type's copy constructor
    var(*other)
{
}

template <underlying_type T, borrow_manager TBorrowManager>
template <borrow_manager TOtherBorrowManager>
var<T, TBorrowManager>::var(const ref<T, TOtherBorrowManager> &other) :
    // The other instance is guaranteed to be valid, because if is a reference
    // Foward the call to the underlying type's copy constructor
    var(*other)
{
}

template <underlying_type T, borrow_manager TBorrowManager>
var<T, TBorrowManager>::var(var<T, TBorrowManager> &&other) noexcept :
    // Always use the saam::var through a reference, so that the reference management can ensure existence of the underlying object
    // Foward the call to the underlying type's move constructor
    var(std::move(*other.borrow()))
{
}

template <underlying_type T, borrow_manager TBorrowManager>
template <borrow_manager TOtherBorrowManager>
var<T, TBorrowManager>::var(var<T, TOtherBorrowManager> &&other) noexcept :
    // Always use the saam::var through a reference, so that the reference management can ensure existence of the underlying object
    // Foward the call to the underlying type's move constructor
    var(std::move(*other.borrow()))
{
}

template <underlying_type T, borrow_manager TBorrowManager>
var<T, TBorrowManager> &var<T, TBorrowManager>::operator=(const var &other)
{
    // Self assignment check is done in the delegated operator=

    // During the assignment, the other instance must be borrowed to ensure proper reference management
    operator=(other.borrow());

    return *this;
}

template <underlying_type T, borrow_manager TBorrowManager>
template <borrow_manager TOtherBorrowManager>
var<T, TBorrowManager> &var<T, TBorrowManager>::operator=(const var<T, TOtherBorrowManager> &other)
{
    // Self assignment check is done in the delegated operator=

    // During the assignment, the other instance must be borrowed to ensure proper reference management
    operator=(other.borrow());

    return *this;
}

template <underlying_type T, borrow_manager TBorrowManager>
var<T, TBorrowManager> &var<T, TBorrowManager>::operator=(const ref<T, TBorrowManager> &other)
{
    // Self assignment check is done in the delegated operator=

    // Only the instance shall be assigned, reference management of "this" and "other" are independent
    operator=(*other.instance_);

    return *this;
}

template <underlying_type T, borrow_manager TBorrowManager>
template <borrow_manager TOtherBorrowManager>
var<T, TBorrowManager> &var<T, TBorrowManager>::operator=(const ref<T, TOtherBorrowManager> &other)
{
    // Self assignment check is done in the delegated operator=

    // Only the instance shall be assigned, reference management of "this" and "other" are independent
    operator=(*other.instance_);

    return *this;
}

template <underlying_type T, borrow_manager TBorrowManager>
var<T, TBorrowManager> &var<T, TBorrowManager>::operator=(var<T, TBorrowManager> &&other) noexcept
{
    // Self assignment check is done in the delegated operator=

    // Only the instance shall be assigned, reference management of "this" and "other" are independent
    operator=(std::move(*(other.borrow().instance_)));

    return *this;
}

template <underlying_type T, borrow_manager TBorrowManager>
template <borrow_manager TOtherBorrowManager>
var<T, TBorrowManager> &var<T, TBorrowManager>::operator=(var<T, TOtherBorrowManager> &&other) noexcept
{
    // Self assignment check is done in the delegated operator=

    // Only the instance shall be assigned, reference management of "this" and "other" are independent
    operator=(std::move(*(other.borrow().instance_)));

    return *this;
}

template <underlying_type T, borrow_manager TBorrowManager>
var<T, TBorrowManager>::~var()
{
    // Pre-destructor offers the possibility to do cleanup before the owned object is destroyed.
    // It is a great place to revoke callbacks that contain self references.
    if constexpr (has_pre_destructor<T>)
    {
        static_assert(has_noexcept_pre_destructor<T>, "pre_destructor() must be noexcept");
        instance_.pre_destructor();
    }

    // Before destroying the owned object, we need to check if there are any active references
    // The desctruction of the owned object cannot be done before the reference check,
    // because it would allow a data race between a thread inside the owned object and the destructor of the owned object.
    if constexpr (has_verify_dangling_references<TBorrowManager>)
    {
        static_assert(has_noexcept_verify_dangling_references<TBorrowManager>, "verify_dangling_references() must be noexcept");
        borrow_manager_.verify_dangling_references(typeid(T), this);
    }
}

template <underlying_type T, borrow_manager TBorrowManager>
ref<T, TBorrowManager> var<T, TBorrowManager>::borrow() const noexcept
{
    T &instance = const_cast<var *>(this)->instance_;
    if constexpr (std::is_same_v<TBorrowManager, unchecked_borrow_manager>)
    {
        return {instance, nullptr};
    }
    else
    {
        return {instance, &borrow_manager_};
    }
}

template <underlying_type T, borrow_manager TBorrowManager>
ref<T, TBorrowManager> var<T, TBorrowManager>::operator->() const noexcept
{
    return borrow();
}

template <underlying_type T, borrow_manager TBorrowManager>
[[nodiscard]] bool var<T, TBorrowManager>::operator==(const T &other) const noexcept
{
    // The borrow lifetime starts before the comparison and ends after it, so the comparison is made with a stable smart reference
    return *borrow() == (other);
}

template <underlying_type T, borrow_manager TBorrowManager>
[[nodiscard]] bool var<T, TBorrowManager>::operator!=(const T &other) const noexcept
{
    // The borrow lifetime starts before the comparison and ends after it, so the comparison is made with a stable smart reference
    return *borrow() != (other);
}

template <underlying_type T, borrow_manager TBorrowManager>
void var<T, TBorrowManager>::call_post_constructor() noexcept
{
    if constexpr (has_post_constructor<T, TBorrowManager>)
    {
        static_assert(has_noexcept_post_constructor<T, TBorrowManager>, "post_constructor() must be noexcept");
        if constexpr (std::is_same_v<TBorrowManager, unchecked_borrow_manager>)
        {
            instance_.post_constructor(ref<T, TBorrowManager>{instance_, nullptr});
        }
        else
        {
            instance_.post_constructor(ref<T, TBorrowManager>{instance_, &borrow_manager_});
        }
    }
}

template <underlying_type T, borrow_manager TBorrowManager>
void var<T, TBorrowManager>::call_post_assignment() noexcept
{
    if constexpr (has_post_assignment<T, TBorrowManager>)
    {
        static_assert(has_noexcept_post_assignment<T, TBorrowManager>, "post_assignment() must be noexcept");
        if constexpr (std::is_same_v<TBorrowManager, unchecked_borrow_manager>)
        {
            instance_.post_assignment(ref<T, TBorrowManager>{instance_, nullptr});
        }
        else
        {
            instance_.post_assignment(ref<T, TBorrowManager>{instance_, &borrow_manager_});
        }
    }
}

}  // namespace saam
