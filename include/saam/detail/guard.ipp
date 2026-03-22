// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/guard.hpp>
#include <saam/safe_ref.hpp>

#include <cassert>

namespace saam
{

//
// Unique guard
//

template <typename T>
template <typename TOther>
    requires std::is_convertible_v<TOther *, T *>
guard<T>::guard(guard<TOther> &&other) noexcept :
    protected_instance_(std::move(other.protected_instance_)),
    lock_(std::move(other.lock_))
{
}

template <typename T>
guard<T>::guard(guard<T> &&other) noexcept :
    protected_instance_(std::move(other.protected_instance_)),
    lock_(std::move(other.lock_))
{
}

template <typename T>
template <typename TOther>
    requires(std::is_convertible_v<TOther *, T *> && !std::is_const_v<TOther>)
guard<T>::guard(const synchronized<TOther> &other) noexcept :
    protected_instance_(other.protected_instance_),
    lock_(*other.active_mutex_)
{
}

template <typename T>
template <typename TOther>
    requires std::is_convertible_v<TOther *, T *>
guard<T> &guard<T>::operator=(guard<TOther> &&other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    protected_instance_ = std::move(other.protected_instance_);
    lock_ = std::move(other.lock_);

    return *this;
}

template <typename T>
guard<T> &guard<T>::operator=(guard<T> &&other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    protected_instance_ = std::move(other.protected_instance_);
    lock_ = std::move(other.lock_);

    return *this;
}

template <typename T>
template <typename TOther>
    requires std::is_convertible_v<TOther *, T *>
guard<T> &guard<T>::operator=(const synchronized<TOther> &other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    protected_instance_ = other.protected_instance_;
    lock_ = lock_t(*other.active_mutex_);

    return *this;
}

template <typename T>
T *guard<T>::operator->() const
{
    return protected_instance_.operator->();
}

template <typename T>
T &guard<T>::operator*() const
{
    return *protected_instance_;
}

template <typename T>
guard<T>::operator T &() const noexcept
{
    return *protected_instance_;
}

template <typename T>
guard<T>::operator T *() const noexcept
{
    return static_cast<T *>(protected_instance_);
}

template <typename T>
template <typename TOther>
    requires(std::is_convertible_v<TOther *, T *> && !std::is_const_v<TOther>)
guard<T>::guard(ref<TOther> protected_instance, lock_t lock) noexcept :
    protected_instance_(std::move(protected_instance)),
    lock_(std::move(lock))
{
    // The mutex is expected to be already locked
}

//
// Shared guard
//

template <typename T>
template <typename TOther>
    requires std::is_convertible_v<TOther *, const T *>
guard<const T>::guard(const guard<const TOther> &other) noexcept :
    protected_instance_(other.protected_instance_)
{
    if (other.lock_.mutex() != nullptr)
    {
        lock_ = lock_t(*other.lock_.mutex());
    }
}

template <typename T>
guard<const T>::guard(const guard<const T> &other) noexcept :
    protected_instance_(other.protected_instance_)
{
    if (other.lock_.mutex() != nullptr)
    {
        lock_ = lock_t(*other.lock_.mutex());
    }
}

template <typename T>
template <typename TOther>
    requires std::is_convertible_v<TOther *, const T *>
guard<const T>::guard(guard<const TOther> &&other) noexcept :
    protected_instance_(std::move(other.protected_instance_)),
    lock_(std::move(other.lock_))
{
}

template <typename T>
guard<const T>::guard(guard<const T> &&other) noexcept :
    protected_instance_(std::move(other.protected_instance_)),
    lock_(std::move(other.lock_))
{
}

template <typename T>
template <typename TOther>
    requires(std::is_convertible_v<TOther *, const T *> && !std::is_const_v<TOther>)
guard<const T>::guard(const synchronized<TOther> &other) noexcept :
    protected_instance_(other.protected_instance_),
    lock_(*other.active_mutex_)
{
}

template <typename T>
template <typename TOther>
    requires std::is_convertible_v<TOther *, const T *>
guard<const T> &guard<const T>::operator=(const guard<const TOther> &other)
{
    if (this == &other)
    {
        return *this;
    }

    protected_instance_ = other.protected_instance_;
    if (other.lock_.mutex() != nullptr)
    {
        lock_ = lock_t(*other.lock_.mutex());
    }

    return *this;
}

template <typename T>
guard<const T> &guard<const T>::operator=(const guard<const T> &other)
{
    if (this == &other)
    {
        return *this;
    }

    protected_instance_ = other.protected_instance_;
    if (other.lock_.mutex() != nullptr)
    {
        lock_ = lock_t(*other.lock_.mutex());
    }

    return *this;
}

template <typename T>
template <typename TOther>
    requires std::is_convertible_v<TOther *, const T *>
guard<const T> &guard<const T>::operator=(guard<const TOther> &&other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    protected_instance_ = std::move(other.protected_instance_);
    lock_ = std::move(other.lock_);

    return *this;
}

template <typename T>
guard<const T> &guard<const T>::operator=(guard<const T> &&other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    protected_instance_ = std::move(other.protected_instance_);
    lock_ = std::move(other.lock_);

    return *this;
}

template <typename T>
template <typename TOther>
    requires std::is_convertible_v<TOther *, const T *>
guard<const T> &guard<const T>::operator=(const synchronized<TOther> &other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    protected_instance_ = other.protected_instance_;
    lock_ = lock_t{*other.active_mutex_};

    return *this;
}

template <typename T>
bool guard<const T>::operator==(const guard &other) const noexcept
{
    return protected_instance_ == other.protected_instance_;
}

template <typename T>
bool guard<const T>::operator!=(const guard &other) const noexcept
{
    return !(*this == other);
}

template <typename T>
const T *guard<const T>::operator->() const
{
    return protected_instance_.operator->();
}

template <typename T>
const T &guard<const T>::operator*() const
{
    return *protected_instance_;
}

template <typename T>
guard<const T>::operator const T &() const noexcept
{
    return *protected_instance_;
}

template <typename T>
guard<const T>::operator const T *() const noexcept
{
    return static_cast<const T *>(protected_instance_);
}

template <typename T>
template <typename TOther>
    requires(std::is_convertible_v<TOther *, T *> && !std::is_const_v<TOther>)
guard<const T>::guard(ref<const TOther> protected_instance, lock_t lock) noexcept :
    protected_instance_(std::move(protected_instance)),
    lock_(std::move(lock))
{
}

}  // namespace saam
