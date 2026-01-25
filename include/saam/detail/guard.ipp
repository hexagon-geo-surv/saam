// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/safe_ref.hpp>
#include <saam/guard.hpp>

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
    mutex_(std::move(other.mutex_))
{
    // The locked state was taken from the other instance - if there was any and the other instance was not moved-from
    // The other instance is now in a moved-from state, will not release the lock
}

template <typename T>
guard<T>::guard(guard<T> &&other) noexcept :
    protected_instance_(std::move(other.protected_instance_)),
    mutex_(std::move(other.mutex_))
{
    // The locked state was taken from the other instance - if there was any and the other instance was not moved-from
    // The other instance is now in a moved-from state, will not release the lock
}

template <typename T>
template <typename TOther>
    requires(std::is_convertible_v<TOther *, T *> && !std::is_const_v<TOther>)
guard<T>::guard(const synchronized<TOther> &other) noexcept :
    protected_instance_(other.protected_instance_),
    mutex_(other.active_mutex_)
{
    lock();
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

    unlock();

    protected_instance_ = std::move(other.protected_instance_);
    mutex_ = std::move(other.mutex_);

    // The locked state was taken from the other instance - if there was any and the other instance was not moved-from
    // The other instance is now in a moved-from state, will not release the lock

    return *this;
}

template <typename T>
guard<T> &guard<T>::operator=(guard<T> &&other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    unlock();

    protected_instance_ = std::move(other.protected_instance_);
    mutex_ = std::move(other.mutex_);

    // The locked state was taken from the other instance - if there was any and the other instance was not moved-from
    // The other instance is now in a moved-from state, will not release the lock

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

    unlock();

    protected_instance_ = other.protected_instance_;
    mutex_ = other.active_mutex_;

    lock();

    return *this;
}

template <typename T>
guard<T>::~guard()
{
    unlock();
}

template <typename T>
void guard<T>::lock() noexcept
{
    if (!mutex_.is_moved_from())
    {
        mutex_->lock();
    }
}

template <typename T>
void guard<T>::unlock() noexcept
{
    if (!mutex_.is_moved_from())
    {
        mutex_->unlock();
    }
}

template <typename T>
bool guard<T>::operator==(const guard &other) const noexcept
{
    return protected_instance_ == other.protected_instance_;
}

template <typename T>
bool guard<T>::operator!=(const guard &other) const noexcept
{
    return !(*this == other);
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

//
// Shared guard
//

template <typename T>
template <typename TOther>
    requires std::is_convertible_v<TOther *, const T *>
guard<const T>::guard(const guard<const TOther> &other) noexcept :
    protected_instance_(other.protected_instance_),
    mutex_(other.mutex_)
{
    lock();
}

template <typename T>
guard<const T>::guard(const guard<const T> &other) noexcept :
    protected_instance_(other.protected_instance_),
    mutex_(other.mutex_)
{
    lock();
}

template <typename T>
template <typename TOther>
    requires std::is_convertible_v<TOther *, const T *>
guard<const T>::guard(guard<const TOther> &&other) noexcept :
    protected_instance_(std::move(other.protected_instance_)),
    mutex_(std::move(other.mutex_))
{
    // The locked state was taken from the other instance - if there was any and the other instance was not moved-from
    // The other instance is now in a moved-from state, will not release the lock
}

template <typename T>
guard<const T>::guard(guard<const T> &&other) noexcept :
    protected_instance_(std::move(other.protected_instance_)),
    mutex_(std::move(other.mutex_))
{
    // The locked state was taken from the other instance - if there was any and the other instance was not moved-from
    // The other instance is now in a moved-from state, will not release the lock
}

template <typename T>
template <typename TOther>
    requires(std::is_convertible_v<TOther *, const T *> && !std::is_const_v<TOther>)
guard<const T>::guard(const synchronized<TOther> &other) noexcept :
    protected_instance_(other.protected_instance_),
    mutex_(other.mutex_)
{
    // The mutex reference in a synchronized is always valid, can be locked without any check
    lock();
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

    unlock();

    protected_instance_ = other.protected_instance_;
    mutex_ = other.mutex_;

    lock();

    return *this;
}

template <typename T>
guard<const T> &guard<const T>::operator=(const guard<const T> &other)
{
    if (this == &other)
    {
        return *this;
    }

    unlock();

    protected_instance_ = other.protected_instance_;
    mutex_ = other.mutex_;

    lock();

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

    unlock();

    protected_instance_ = std::move(other.protected_instance_);
    mutex_ = std::move(other.mutex_);

    // The locked state was taken from the other instance - if there was any and the other instance was not moved-from
    // The other instance is now in a moved-from state, will not release the lock

    return *this;
}

template <typename T>
guard<const T> &guard<const T>::operator=(guard<const T> &&other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    unlock();

    protected_instance_ = std::move(other.protected_instance_);
    mutex_ = std::move(other.mutex_);

    // The locked state was taken from the other instance - if there was any and the other instance was not moved-from
    // The other instance is now in a moved-from state, will not release the lock

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

    unlock();

    protected_instance_ = other.protected_instance_;
    mutex_ = other.active_mutex_;

    // The mutex reference in a synchronized is always valid, can be locked without any check
    lock();

    return *this;
}

template <typename T>
guard<const T>::~guard()
{
    unlock();
}

template <typename T>
void guard<const T>::lock() noexcept
{
    if (!mutex_.is_moved_from())
    {
        mutex_->lock_shared();
    }
}

template <typename T>
void guard<const T>::unlock() noexcept
{
    if (!mutex_.is_moved_from())
    {
        mutex_->unlock_shared();
    }
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

}  // namespace saam
