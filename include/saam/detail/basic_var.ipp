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
basic_var<T, TBorrowManager>::basic_var(Args &&...args) :
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
    basic_var(other.instance_)
{
    // Just take the other instance, reference counters of "this" and "other" are independent
}

template <typename T, borrow_manager TBorrowManager>
basic_var<T, TBorrowManager>::basic_var(basic_var &&other) noexcept :
    basic_var(std::move(other.instance_))
{
    // Just take the other instance, reference counters of "this" and "other" are independent
}

template <typename T, borrow_manager TBorrowManager>
basic_var<T, TBorrowManager> &basic_var<T, TBorrowManager>::operator=(const basic_var &other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    // Just take the other instance, reference management of "this" and "other" are independent
    instance_ = other.instance_;
    return *this;
}

template <typename T, borrow_manager TBorrowManager>
basic_var<T, TBorrowManager> &basic_var<T, TBorrowManager>::operator=(basic_var &&other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    // Just take the other instance, reference management of "this" and "other" are independent
    instance_ = std::move(other.instance_);
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
