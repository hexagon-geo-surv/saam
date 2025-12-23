// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/safe_ref.hpp>

#include <mutex>
#include <shared_mutex>

namespace saam
{

template <typename T>
    requires(!std::is_const_v<T>)
class synchronized;

// Provides access to the protected instance of T, which is protected by a synchronized.
// The sentinel is expected to be a short living object, its lifetime is often bound to scope or it is a temporal variable. It is not
// supposed to be stored for a long time.
template <typename T>
class sentinel
{
  public:
    using value_t = T;

    // Unique sentinel is unique, after the copy we would have two unique sentinels, which is not unique anymore
    sentinel(const sentinel<T> &other) = delete;

    // Conversion move constructor
    template <typename TOther>
        requires std::is_convertible_v<TOther *, T *>
    sentinel(sentinel<TOther> &&other) noexcept :
        protected_instance_(std::move(other.protected_instance_)),
        lock_(std::move(other.lock_))
    {
    }

    // No conversion upgrade (immutable to mutable) move constructor.
    template <typename TOther>
        requires std::is_convertible_v<TOther *, T *>
    sentinel(sentinel<const TOther> &&other) noexcept = delete;

    // The conversion move constructor does not cover the move constructor, so we need to implement it explicitly
    sentinel(sentinel<T> &&other) noexcept :
        protected_instance_(std::move(other.protected_instance_)),
        lock_(std::move(other.lock_))
    {
    }

    // No upgrade (immutable to mutable) move constructor.
    sentinel(sentinel<const T> &&other) noexcept = delete;

    // Conversion copy construction from synchronized
    template <typename TOther>
        requires(std::is_convertible_v<TOther *, T *> && !std::is_const_v<TOther>)
    sentinel(const synchronized<TOther> &other) noexcept;

    // Unique sentinel is unique, after the assigment we would have two unique sentinels
    sentinel &operator=(const sentinel<T> &other) = delete;

    // Conversion move assignment operator from sentinel
    template <typename TOther>
        requires std::is_convertible_v<TOther *, T *>
    sentinel &operator=(sentinel<TOther> &&other) noexcept
    {
        protected_instance_ = std::move(other.protected_instance_);
        lock_ = std::move(other.lock_);

        return *this;
    }

    // No conversion upgrade (immutable to mutable) move assignment.
    template <typename TOther>
        requires std::is_convertible_v<TOther *, T *>
    sentinel &operator=(sentinel<const TOther> &&other) noexcept = delete;

    sentinel &operator=(sentinel<T> &&other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }

        protected_instance_ = std::move(other.protected_instance_);
        lock_ = std::move(other.lock_);

        return *this;
    }

    // No upgrade (immutable to mutable) move assignment.
    sentinel &operator=(sentinel<const T> &&other) noexcept = delete;

    // Conversion assignment operator from synchronized
    template <typename TOther>
        requires std::is_convertible_v<TOther *, T *>
    sentinel &operator=(const synchronized<TOther> &other) noexcept;

    ~sentinel() = default;

    // Arrow operator
    [[nodiscard]] T *operator->() const
    {
        return protected_instance_.operator->();
    }

    // Dereference operator
    [[nodiscard]] T &operator*() const
    {
        return *protected_instance_;
    }

    // Cast to T reference
    [[nodiscard]] explicit operator T &() const noexcept
    {
        return *protected_instance_;
    }

    // Cast to T pointer
    [[nodiscard]] explicit operator T *() const noexcept
    {
        return static_cast<T *>(protected_instance_);
    }

  private:
    template <typename TOther>
        requires(!std::is_const_v<TOther>)
    friend class synchronized;

    template <typename TOther>
    friend class sentinel;

    friend class condition;

    void unlock()
    {
        if (lock_.owns_lock())
        {
            lock_.unlock();
        }
    }

    sentinel(saam::ref<T> instance, std::unique_lock<std::shared_mutex> lock) :
        protected_instance_(std::move(instance)),
        lock_(std::move(lock))
    {
    }

    saam::ref<T> protected_instance_;
    std::unique_lock<std::shared_mutex> lock_;
};

// The underlying lock is a shared lock for const access, so a separate implementation is needed.
template <typename T>
class sentinel<const T>
{
  public:
    using value_t = const T;

    // Conversion copy constructor
    template <typename TOther>
        requires std::is_convertible_v<TOther *, const T *>
    sentinel(const sentinel<const TOther> &other) noexcept :
        protected_instance_(other.protected_instance_),
        lock_(other.shared_lock_)
    {
    }

    // No conversion downgrade (mutable to immutable) copy constructor.
    template <typename TOther>
        requires std::is_convertible_v<TOther *, const T *>
    sentinel(const sentinel<TOther> &other) noexcept = delete;

    // The conversion copy constructor does not cover the copy constructor, so we need to implement it explicitly
    sentinel(const sentinel<const T> &other) :
        protected_instance_(other.protected_instance_),
        lock_(*other.lock_.mutex())
    {
    }

    // No downgrade (mutable to immutable) copy constructor.
    sentinel(const sentinel<T> &other) noexcept = delete;

    // Conversion move constructor
    template <typename TOther>
        requires std::is_convertible_v<TOther *, const T *>
    sentinel(sentinel<const TOther> &&other) noexcept :
        protected_instance_(std::move(other.protected_instance_)),
        lock_(std::move(other.shared_lock_))
    {
    }

    // No conversion downgrade (mutable to immutable) move constructor.
    template <typename TOther>
        requires std::is_convertible_v<TOther *, const T *>
    sentinel(sentinel<TOther> &&other) noexcept = delete;

    // The conversion move constructor does not cover the move constructor, so we need to implement it explicitly
    sentinel(sentinel<const T> &&other) noexcept :
        protected_instance_(std::move(other.protected_instance_)),
        lock_(std::move(other.lock_))
    {
    }

    // No downgrade (mutable to immutable) move constructor.
    sentinel(sentinel<T> &&other) noexcept = delete;

    // Conversion copy construction from synchronized
    template <typename TOther>
        requires(std::is_convertible_v<TOther *, const T *> && !std::is_const_v<TOther>)
    sentinel(const synchronized<TOther> &other) noexcept;

    // Conversion copy assignment operator from sentinel
    template <typename TOther>
        requires std::is_convertible_v<TOther *, const T *>
    sentinel &operator=(const sentinel<const TOther> &other)
    {
        protected_instance_ = other.protected_instance_;
        lock_ = other.shared_lock_;

        return *this;
    }

    // No conversion downgrade (mutable to immutable) copy assignment.
    template <typename TOther>
        requires std::is_convertible_v<TOther *, const T *>
    sentinel &operator=(const sentinel<TOther> &other) = delete;

    sentinel &operator=(const sentinel<const T> &other)
    {
        if (this == &other)
        {
            return *this;
        }

        protected_instance_ = other.protected_instance_;
        lock_ = std::shared_lock(*other.lock_.mutex());

        return *this;
    }

    // No downgrade (mutable to immutable) copy assignment.
    sentinel &operator=(const sentinel<T> &other) = delete;

    // Conversion move assignment operator from sentinel
    template <typename TOther>
        requires std::is_convertible_v<TOther *, const T *>
    sentinel &operator=(sentinel<const TOther> &&other) noexcept
    {
        protected_instance_ = std::move(other.protected_instance_);
        lock_ = std::move(other.shared_lock_);

        return *this;
    }

    // No conversion downgrade (mutable to immutable) move assignment.
    template <typename TOther>
        requires std::is_convertible_v<TOther *, const T *>
    sentinel &operator=(sentinel<TOther> &&other) noexcept = delete;

    sentinel &operator=(sentinel<const T> &&other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }

        protected_instance_ = std::move(other.protected_instance_);
        lock_ = std::move(other.lock_);

        return *this;
    }

    // No downgrade (mutable to immutable) move assignment.
    sentinel &operator=(sentinel<T> &&other) noexcept = delete;

    // Conversion assignment operator from synchronized
    template <typename TOther>
        requires std::is_convertible_v<TOther *, const T *>
    sentinel &operator=(const synchronized<TOther> &other) noexcept;

    ~sentinel() = default;

    // Arrow operator
    [[nodiscard]] const T *operator->() const
    {
        return protected_instance_.operator->();
    }

    // Dereference operator
    [[nodiscard]] const T &operator*() const
    {
        return *protected_instance_;
    }

    // Cast to T reference
    [[nodiscard]] explicit operator const T &() const noexcept
    {
        return *protected_instance_;
    }

    // Cast to T pointer
    [[nodiscard]] explicit operator const T *() const noexcept
    {
        return static_cast<const T *>(protected_instance_);
    }


  private:
    template <typename TOther>
        requires(!std::is_const_v<TOther>)
    friend class synchronized;

    template <typename TOther>
    friend class sentinel;

    friend class condition;

    void unlock()
    {
        if (lock_.owns_lock())
        {
            lock_.unlock();
        }
    }

    sentinel(saam::ref<const T> instance, std::shared_lock<std::shared_mutex> lock) :
        protected_instance_(std::move(instance)),
        lock_(std::move(lock))
    {
    }

    saam::ref<const T> protected_instance_;
    std::shared_lock<std::shared_mutex> lock_;
};

}  // namespace saam
