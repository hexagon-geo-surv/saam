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
    protected_instance_(std::in_place, std::forward<Args>(args)...)
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
    // Copy over the content of the other protected instance
    protected_instance_(*other.commence())
{
    // The mutexes of "this" and "other" are independent
}

template <typename T>
    requires(!std::is_const_v<T>)
synchronized<T>::synchronized(synchronized &&other) :
    // Move over the content of the other protected instance
    protected_instance_(std::move(*other.commence_mut()))
{
    // The mutexes of "this" and "other" are independent
}

template <typename T>
    requires(!std::is_const_v<T>)
synchronized<T> &synchronized<T>::operator=(const synchronized<T> &other)
{
    if (this == &other)
    {
        return *this;
    }

    std::unique_lock<mutex_t> this_lock(mutex_, std::defer_lock);
    std::shared_lock<mutex_t> other_lock(other.mutex_, std::defer_lock);
    std::lock(this_lock, other_lock);

    protected_instance_ = other.protected_instance_;

    return *this;
}

template <typename T>
    requires(!std::is_const_v<T>)
synchronized<T> &synchronized<T>::operator=(synchronized<T> &&other)
{
    if (this == &other)
    {
        return *this;
    }

    std::unique_lock<mutex_t> this_lock(mutex_, std::defer_lock);
    std::unique_lock<mutex_t> other_lock(other.mutex_, std::defer_lock);
    std::lock(this_lock, other_lock);

    protected_instance_ = std::move(other.protected_instance_);

    return *this;
}

template <typename T>
    requires(!std::is_const_v<T>)
synchronized<T>::~synchronized() = default;

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
[[nodiscard]] guard<T> synchronized<T>::operator->() const noexcept
{
    return guard<T>(*this);
}

template <typename T>
    requires(!std::is_const_v<T>)
synchronized<T> &synchronized<T>::operator=(const T &instance)
{
    *commence_mut() = instance;
    return *this;
}

template <typename T>
    requires(!std::is_const_v<T>)
synchronized<T> &synchronized<T>::operator=(T &&instance)
{
    *commence_mut() = std::move(instance);
    return *this;
}

}  // namespace saam
