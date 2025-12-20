// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/safe_ref.hpp>
#include <saam/sentinel.hpp>

#include <mutex>
#include <shared_mutex>
#include <type_traits>
#include <utility>

namespace saam
{

// Synchronized owns an instance of T and provides a thread-safe way to access it.
// The lifetime of T is bound to the lifetime of the synchronized.
template <typename T>
    requires(!std::is_const_v<T>)  // Only mutable types need synchronization
class synchronized
{
  public:
    synchronized() = default;

    template <typename... Args>
    explicit synchronized(std::in_place_t, Args &&...args) :
        protected_instance_(std::forward<Args>(args)...)
    {
    }

    explicit synchronized(const T &instance) :
        protected_instance_(instance)
    {
    }

    explicit synchronized(T &&instance) :
        protected_instance_(std::move(instance))
    {
    }

    // No conversion copy constructor, because of the slicing of T - only the base class would be copied
    synchronized(const synchronized &other) :
        protected_instance_(*other.lock())
    {
        // Just take the other instance, synchronizedes of "this" and "other" are independent
    }

    // No conversion move constructor, because of the slicing of T - only the base class would be copied
    synchronized(synchronized &&other) noexcept :
        protected_instance_(std::move(*other.lock_mut()))
    {
        // Just take the other instance, synchronizedes of "this" and "other" are independent
    }

    // No conversion copy assignment, because of the slicing of T - only the base class would be copied
    synchronized &operator=(const synchronized &other)
    {
        if (this == &other)
        {
            return *this;
        }

        std::scoped_lock lock(protector_mutex_, other.protector_mutex_);

        // Just take the other instance, synchronizedes of "this" and "other" are independent
        protected_instance_ = other.protected_instance_;

        return *this;
    }

    // No conversion move assignment, because of the slicing of T - only the base class would be copied
    synchronized &operator=(synchronized &&other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }

        std::scoped_lock lock(protector_mutex_, other.protector_mutex_);

        // Just take the other instance, mutexes of "this" and "other" are independent
        protected_instance_ = std::move(other.protected_instance_);

        return *this;
    }

    // No race condition is possible during destruction
    // If another thread is in the class then it must have a smart reference (ref instance),
    // so the smart owner (var instance) does not let wrapped the synchronized deleted destructed
    ~synchronized() = default;

    // All borrowings are const operations, because borrowing just provides access to the underlying object - does not change the managed
    // wrapper.

    // Mutable borrow
    template <typename TMut = T>
        requires(!std::is_const_v<TMut>)
    [[nodiscard]] sentinel<T> lock_mut() const
    {
        return sentinel<T>(const_cast<synchronized *>(this)->protected_instance_, std::unique_lock(protector_mutex_));
    }

    // Immutable borrow
    [[nodiscard]] sentinel<const T> lock() const
    {
        return sentinel<const T>(protected_instance_, std::shared_lock(protector_mutex_));
    }

  private:
    template <typename TOther>
        requires(!std::is_const_v<TOther>)
    friend class synchronized;

    template <typename TOther>
    friend class sentinel;

    friend class condition;

    mutable std::shared_mutex protector_mutex_;

    saam::var<T> protected_instance_;
};

template <typename T>
template <typename TOther>
    requires(std::is_convertible_v<TOther *, T *> && !std::is_const_v<TOther>)
sentinel<T>::sentinel(const synchronized<TOther> &other) noexcept :
    sentinel(const_cast<synchronized<TOther> &>(other).protected_instance_,
             std::unique_lock(const_cast<synchronized<TOther> &>(other).protector_mutex_))
{
}

template <typename T>
template <typename TOther>
    requires(std::is_convertible_v<TOther *, const T *> && !std::is_const_v<TOther>)
sentinel<const T>::sentinel(const synchronized<TOther> &other) noexcept :
    sentinel(const_cast<synchronized<TOther> &>(other).protected_instance_,
             std::shared_lock(const_cast<synchronized<TOther> &>(other).protector_mutex_))
{
}

template <typename T>
template <typename TOther>
    requires std::is_convertible_v<TOther *, T *>
sentinel<T> &sentinel<T>::operator=(const synchronized<TOther> &other) noexcept
{
    protected_instance_ = other.protected_instance_;

    lock_ = std::unique_lock(other.protector_mutex_);

    return *this;
}

template <typename T>
template <typename TOther>
    requires std::is_convertible_v<TOther *, const T *>
sentinel<const T> &sentinel<const T>::operator=(const synchronized<TOther> &other) noexcept
{
    protected_instance_ = other.protected_instance_;

    lock_ = std::shared_lock(other.protector_mutex_);

    return *this;
}

}  // namespace saam
