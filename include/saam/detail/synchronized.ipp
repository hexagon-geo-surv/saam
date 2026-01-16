// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/synchronized.hpp>

namespace saam
{

template <typename T>
    requires(!std::is_const_v<T>)
synchronized<T>::synchronized() = default;

template <typename T>
    requires(!std::is_const_v<T>)
template <typename... Args>
synchronized<T>::synchronized(std::in_place_t, Args &&...args) :
    protected_instance_(std::forward<Args>(args)...)
{
}

template <typename T>
    requires(!std::is_const_v<T>)
synchronized<T>::synchronized(const T &instance) :
    protected_instance_(instance)
{
}

template <typename T>
    requires(!std::is_const_v<T>)
synchronized<T>::synchronized(T &&instance) :
    protected_instance_(std::move(instance))
{
}

template <typename T>
    requires(!std::is_const_v<T>)
synchronized<T>::synchronized(const synchronized &other) :
    protected_instance_(*other.lock())
{
    // Just take the other instance, synchronizedes of "this" and "other" are independent
}

template <typename T>
    requires(!std::is_const_v<T>)
synchronized<T>::synchronized(synchronized &&other) noexcept :
    protected_instance_(std::move(*other.lock_mut()))
{
    // Just take the other instance, synchronizedes of "this" and "other" are independent
}
template <typename T>
    requires(!std::is_const_v<T>)
synchronized<T> &synchronized<T>::operator=(const synchronized<T> &other)
{
    if (this == &other)
    {
        return *this;
    }

    // std::scoped_lock lock(protector_mutex_, other.protector_mutex_);

    // Just take the other instance, synchronizedes of "this" and "other" are independent
    protected_instance_ = other.protected_instance_;

    return *this;
}

template <typename T>
    requires(!std::is_const_v<T>)
synchronized<T> &synchronized<T>::operator=(synchronized<T> &&other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    // std::scoped_lock lock(protector_mutex_, other.protector_mutex_);

    // Just take the other instance, mutexes of "this" and "other" are independent
    protected_instance_ = std::move(other.protected_instance_);

    return *this;
}

template <typename T>
    requires(!std::is_const_v<T>)
synchronized<T>::~synchronized() = default;

template <typename T>
    requires(!std::is_const_v<T>)
template <typename TOther>
synchronized<T> &synchronized<T>::use_mutex_of(ref<synchronized<TOther>> other)
{
    active_mutex_ = std::move(other->mutex_);
    return *this;
}

template <typename T>
    requires(!std::is_const_v<T>)
sentinel<T> synchronized<T>::lock_mut() const
{
    return sentinel<T>(*this);
}

template <typename T>
    requires(!std::is_const_v<T>)

sentinel<const T> synchronized<T>::lock() const
{
    return sentinel<const T>(*this);
}

}  // namespace saam
