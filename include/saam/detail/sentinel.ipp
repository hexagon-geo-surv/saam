// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/safe_ref.hpp>
#include <saam/sentinel.hpp>

#include <cassert>

namespace saam
{

//
// Unique sentinel
//

template <typename T>
template <typename TOther>
    requires std::is_convertible_v<TOther *, T *>
sentinel<T>::sentinel(sentinel<TOther> &&other) noexcept :
    protected_instance_(std::move(other.protected_instance_)),
    mutex_(std::move(other.mutex_))
{
    // The locked state was taken from the other instance - if there was any and the other instance was not moved-from
    // The other instance is now in a moved-from state, will not release the lock
}

template <typename T>
sentinel<T>::sentinel(sentinel<T> &&other) noexcept :
    protected_instance_(std::move(other.protected_instance_)),
    mutex_(std::move(other.mutex_))
{
    // The locked state was taken from the other instance - if there was any and the other instance was not moved-from
    // The other instance is now in a moved-from state, will not release the lock
}

template <typename T>
template <typename TOther>
    requires(std::is_convertible_v<TOther *, T *> && !std::is_const_v<TOther>)
sentinel<T>::sentinel(const synchronized<TOther> &other) noexcept :
    protected_instance_(other.protected_instance_),
    mutex_(other.active_mutex_)
{
    // The mutex reference in a synchronized is always valid, can be locked without any check
    mutex_->lock();
}

template <typename T>
template <typename TOther>
    requires std::is_convertible_v<TOther *, T *>
sentinel<T> &sentinel<T>::operator=(sentinel<TOther> &&other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    if (!mutex_.is_moved_from())
    {
        mutex_->unlock();
    }

    protected_instance_ = std::move(other.protected_instance_);
    mutex_ = std::move(other.mutex_);

    // The locked state was taken from the other instance - if there was any and the other instance was not moved-from
    // The other instance is now in a moved-from state, will not release the lock

    return *this;
}

template <typename T>
sentinel<T> &sentinel<T>::operator=(sentinel<T> &&other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    if (!mutex_.is_moved_from())
    {
        mutex_->unlock();
    }

    protected_instance_ = std::move(other.protected_instance_);
    mutex_ = std::move(other.mutex_);

    // The locked state was taken from the other instance - if there was any and the other instance was not moved-from
    // The other instance is now in a moved-from state, will not release the lock

    return *this;
}

template <typename T>
template <typename TOther>
    requires std::is_convertible_v<TOther *, T *>
sentinel<T> &sentinel<T>::operator=(const synchronized<TOther> &other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    if (!mutex_.is_moved_from())
    {
        mutex_->unlock();
    }

    protected_instance_ = other.protected_instance_;
    mutex_ = other.active_mutex_;

    // The mutex reference in a syhcronized is always valid, can be locked without any check
    mutex_->lock();

    return *this;
}

template <typename T>
sentinel<T>::~sentinel()
{
    if (!mutex_.is_moved_from())
    {
        mutex_->unlock();
    }
}

template <typename T>
bool sentinel<T>::operator==(const sentinel &other) const noexcept
{
    return protected_instance_ == other.protected_instance_;
}

template <typename T>
bool sentinel<T>::operator!=(const sentinel &other) const noexcept
{
    return !(*this == other);
}

template <typename T>
T *sentinel<T>::operator->() const
{
    return protected_instance_.operator->();
}

template <typename T>
T &sentinel<T>::operator*() const
{
    return *protected_instance_;
}

template <typename T>
sentinel<T>::operator T &() const noexcept
{
    return *protected_instance_;
}

template <typename T>
sentinel<T>::operator T *() const noexcept
{
    return static_cast<T *>(protected_instance_);
}

//
// Shared sentinel
//

template <typename T>
template <typename TOther>
    requires std::is_convertible_v<TOther *, const T *>
sentinel<const T>::sentinel(const sentinel<const TOther> &other) noexcept :
    protected_instance_(other.protected_instance_),
    mutex_(other.mutex_)
{
    if (!mutex_.is_moved_from())
    {
        mutex_->lock_shared();
    }
}

template <typename T>
sentinel<const T>::sentinel(const sentinel<const T> &other) noexcept :
    protected_instance_(other.protected_instance_),
    mutex_(other.mutex_)
{
    if (!mutex_.is_moved_from())
    {
        mutex_->lock_shared();
    }
}

template <typename T>
template <typename TOther>
    requires std::is_convertible_v<TOther *, const T *>
sentinel<const T>::sentinel(sentinel<const TOther> &&other) noexcept :
    protected_instance_(std::move(other.protected_instance_)),
    mutex_(std::move(other.mutex_))
{
    // The locked state was taken from the other instance - if there was any and the other instance was not moved-from
    // The other instance is now in a moved-from state, will not release the lock
}

template <typename T>
sentinel<const T>::sentinel(sentinel<const T> &&other) noexcept :
    protected_instance_(std::move(other.protected_instance_)),
    mutex_(std::move(other.mutex_))
{
    // The locked state was taken from the other instance - if there was any and the other instance was not moved-from
    // The other instance is now in a moved-from state, will not release the lock
}

template <typename T>
template <typename TOther>
    requires(std::is_convertible_v<TOther *, const T *> && !std::is_const_v<TOther>)
sentinel<const T>::sentinel(const synchronized<TOther> &other) noexcept :
    protected_instance_(other.protected_instance_),
    mutex_(other.mutex_)
{
    // The mutex reference in a synchronized is always valid, can be locked without any check
    mutex_->lock_shared();
}

template <typename T>
template <typename TOther>
    requires std::is_convertible_v<TOther *, const T *>
sentinel<const T> &sentinel<const T>::operator=(const sentinel<const TOther> &other)
{
    if (this == &other)
    {
        return *this;
    }

    if (!mutex_.is_moved_from())
    {
        mutex_->unlock_shared();
    }

    protected_instance_ = other.protected_instance_;
    mutex_ = other.mutex_;

    if (!mutex_.is_moved_from())
    {
        mutex_->lock_shared();
    }

    return *this;
}

template <typename T>
sentinel<const T> &sentinel<const T>::operator=(const sentinel<const T> &other)
{
    if (this == &other)
    {
        return *this;
    }

    if (!mutex_.is_moved_from())
    {
        mutex_->unlock_shared();
    }

    protected_instance_ = other.protected_instance_;
    mutex_ = other.mutex_;

    if (!mutex_.is_moved_from())
    {
        mutex_->lock_shared();
    }

    return *this;
}

template <typename T>
template <typename TOther>
    requires std::is_convertible_v<TOther *, const T *>
sentinel<const T> &sentinel<const T>::operator=(sentinel<const TOther> &&other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    if (!mutex_.is_moved_from())
    {
        mutex_->unlock_shared();
    }

    protected_instance_ = std::move(other.protected_instance_);
    mutex_ = std::move(other.mutex_);

    // The locked state was taken from the other instance - if there was any and the other instance was not moved-from
    // The other instance is now in a moved-from state, will not release the lock

    return *this;
}

template <typename T>
sentinel<const T> &sentinel<const T>::operator=(sentinel<const T> &&other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    if (!mutex_.is_moved_from())
    {
        mutex_->unlock_shared();
    }

    protected_instance_ = std::move(other.protected_instance_);
    mutex_ = std::move(other.mutex_);

    // The locked state was taken from the other instance - if there was any and the other instance was not moved-from
    // The other instance is now in a moved-from state, will not release the lock

    return *this;
}

template <typename T>
template <typename TOther>
    requires std::is_convertible_v<TOther *, const T *>
sentinel<const T> &sentinel<const T>::operator=(const synchronized<TOther> &other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    if (!mutex_.is_moved_from())
    {
        mutex_->unlock_shared();
    }

    protected_instance_ = other.protected_instance_;
    mutex_ = other.active_mutex_;

    // The mutex reference in a syhcronized is always valid, can be locked without any check
    mutex_->lock_shared();

    return *this;
}

template <typename T>
sentinel<const T>::~sentinel()
{
    if (!mutex_.is_moved_from())
    {
        mutex_->unlock_shared();
    }
}

template <typename T>
bool sentinel<const T>::operator==(const sentinel &other) const noexcept
{
    return protected_instance_ == other.protected_instance_;
}

template <typename T>
bool sentinel<const T>::operator!=(const sentinel &other) const noexcept
{
    return !(*this == other);
}

template <typename T>
const T *sentinel<const T>::operator->() const
{
    return protected_instance_.operator->();
}

template <typename T>
const T &sentinel<const T>::operator*() const
{
    return *protected_instance_;
}

template <typename T>
sentinel<const T>::operator const T &() const noexcept
{
    return *protected_instance_;
}

template <typename T>
sentinel<const T>::operator const T *() const noexcept
{
    return static_cast<const T *>(protected_instance_);
}

}  // namespace saam
