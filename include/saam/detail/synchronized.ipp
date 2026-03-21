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
    protected_instance_(*other.commence())
{
    // Just take the other instance, synchronizedes of "this" and "other" are independent
}

template <typename T>
    requires(!std::is_const_v<T>)
synchronized<T>::synchronized(synchronized &&other) noexcept :
    protected_instance_(std::move(*other.commence_mut()))
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
guard<T> synchronized<T>::commence_mut() const
{
    return guard<T>(*this);
}

template <typename T>
    requires(!std::is_const_v<T>)
guard<const T> synchronized<T>::commence() const
{
    return guard<const T>(*this);
}

template <typename T>
    requires(!std::is_const_v<T>)
template <typename... Args>
synchronized<T> &synchronized<T>::emplace(Args &&...args)
{
    *commence_mut() = T(std::forward<Args>(args)...);
    return *this;
}

template <typename T>
    requires(!std::is_const_v<T>)
[[nodiscard]] guard<T> synchronized<T>::operator->() const noexcept
{
    return guard<T>(*this);
}

template <typename T>
    requires(!std::is_const_v<T>)
synchronized<T> &synchronized<T>::operator=(const T &instance) noexcept
{
    *commence_mut() = instance;
    return *this;
}

template <typename T>
    requires(!std::is_const_v<T>)
synchronized<T> &synchronized<T>::operator=(T &&instance) noexcept
{
    *commence_mut() = std::move(instance);
    return *this;
}

}  // namespace saam
