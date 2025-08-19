// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/detail/basic_var.hpp>
#include <saam/detail/constructor_destructor_traits.hpp>

#include <type_traits>
#include <utility>

namespace saam
{

template <class T, borrow_manager TBorrowManager>
basic_var<T, TBorrowManager>::basic_var()
{
    configure_enable_ref_from_this();
    call_post_constructor();
}

template <class T, borrow_manager TBorrowManager>
template <typename... Args>
basic_var<T, TBorrowManager>::basic_var(std::in_place_t, Args &&...args)
    : instance_(std::forward<Args>(args)...)
{
    configure_enable_ref_from_this();
    call_post_constructor();
}

template <class T, borrow_manager TBorrowManager>
basic_var<T, TBorrowManager>::basic_var(const T &instance)
    : instance_(instance)
{
    configure_enable_ref_from_this();
    call_post_constructor();
}

template <class T, borrow_manager TBorrowManager>
basic_var<T, TBorrowManager>::basic_var(T &&instance)
    : instance_(std::move(instance))
{
    configure_enable_ref_from_this();
    call_post_constructor();
}

template <class T, borrow_manager TBorrowManager>
basic_var<T, TBorrowManager>::basic_var(const basic_var &other)
    : basic_var(other.instance_)
{
    // Just take the other instance, reference counters of "this" and "other" are independent
}

template <class T, borrow_manager TBorrowManager>
basic_var<T, TBorrowManager>::basic_var(basic_var &&other) noexcept
    : basic_var(std::move(other.instance_))
{
    // Just take the other instance, reference counters of "this" and "other" are independent
}

template <class T, borrow_manager TBorrowManager>
basic_var<T, TBorrowManager> &basic_var<T, TBorrowManager>::operator=(const basic_var &other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    // Just take the other instance, reference management of "this" and "other" are independent
    // enable_ref_from_this support is managed by the instance itself via the enable_ref_from_this base class
    instance_ = other.instance_;
    return *this;
}

template <class T, borrow_manager TBorrowManager>
basic_var<T, TBorrowManager> &basic_var<T, TBorrowManager>::operator=(basic_var &&other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    // Just take the other instance, reference management of "this" and "other" are independent
    // enable_ref_from_this support is managed by the instance itself via the enable_ref_from_this base class
    instance_ = std::move(other.instance_);
    return *this;
}

template <class T, borrow_manager TBorrowManager>
basic_var<T, TBorrowManager>::~basic_var()
{
    // Pre-destructor offers the possibility to do cleanup before the owned object is destroyed.
    // It is a great place to revoke callbacks that contain self references.
    call_pre_destructor();

    // Before destroying the owned object, we need to check if there are any active references
    // The desctruction of the owned object cannot be done before the reference check,
    // because it would allow a data race between a thread inside the owned object and the destructor of the owned object.
    borrow_manager_.verify_dangling_references(typeid(T));
}

template <class T, borrow_manager TBorrowManager>
basic_ref<T, TBorrowManager> basic_var<T, TBorrowManager>::borrow() const noexcept
{
    return basic_ref<T, TBorrowManager>(const_cast<basic_var *>(this)->instance_, borrow_manager_);
}

template <class T, borrow_manager TBorrowManager>
void basic_var<T, TBorrowManager>::call_pre_destructor()
{
    if constexpr (has_pre_destructor<T>)
    {
        instance_.pre_destructor();
    }
}

template <class T, borrow_manager TBorrowManager>
void basic_var<T, TBorrowManager>::configure_enable_ref_from_this()
{
    if constexpr (std::is_base_of_v<basic_enable_ref_from_this<T, TBorrowManager>, T>)
    {
        instance_.smart_variable(this);
    }
}

template <class T, borrow_manager TBorrowManager>
void basic_var<T, TBorrowManager>::call_post_constructor()
{
    if constexpr (has_post_constructor<T>)
    {
        instance_.post_constructor();
    }
}

}  // namespace saam
