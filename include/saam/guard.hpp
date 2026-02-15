// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/safe_ref.hpp>
#include <saam/shared_recursive_mutex.hpp>

namespace saam
{

template <typename T>
    requires(!std::is_const_v<T>)
class synchronized;

// Provides access to the protected instance of T, which is protected by a synchronized.
// The guard is expected to be a short living object, its lifetime is often bound to scope or it is a temporal variable. It is not
// supposed to be stored for a long time.
template <typename T>
class guard
{
  public:
    using value_t = T;

    class blindfold
    {
      public:
        explicit blindfold(guard &guard) :
            original_guard_(&guard),
            unlocked_guard_(std::move([](auto &&sent) {
                sent.unlock();
                return std::forward<decltype(sent)>(sent);
            }(std::move(guard))))
        {
        }

        blindfold(const blindfold &other) = delete;
        blindfold(blindfold &&other) :
            original_guard_(other.original_guard_),
            unlocked_guard_(std::move(other.unlocked_guard_))
        {
            other.original_guard_ = nullptr;
        }

        blindfold &operator=(const blindfold &other) = delete;
        blindfold &operator=(blindfold &&other)
        {
            if (this == &other)
            {
                return *this;
            }

            unblind_original_guard();

            original_guard_ = other.original_guard_;
            unlocked_guard_ = std::move(other.unlocked_guard_);
            other.original_guard_ = nullptr;

            return *this;
        }

        ~blindfold()
        {
            unblind_original_guard();
        }

      private:
        void unblind_original_guard()
        {
            if (original_guard_ != nullptr)
            {
                *original_guard_ = std::move(unlocked_guard_);
                original_guard_->lock();
                original_guard_ = nullptr;
            }
        }

        guard *original_guard_;
        guard unlocked_guard_;
    };

    // Unique guard is unique, after the copy we would have two unique guards, which is not unique anymore
    guard(const guard<T> &other) = delete;

    // Conversion move constructor
    template <typename TOther>
        requires std::is_convertible_v<TOther *, T *>
    guard(guard<TOther> &&other) noexcept;

    // No conversion upgrade (immutable to mutable) move constructor.
    template <typename TOther>
        requires std::is_convertible_v<TOther *, T *>
    guard(guard<const TOther> &&other) noexcept = delete;

    // The conversion move constructor does not cover the move constructor, so we need to implement it explicitly
    guard(guard<T> &&other) noexcept;

    // No upgrade (immutable to mutable) move constructor.
    guard(guard<const T> &&other) noexcept = delete;

    // Conversion copy construction from synchronized
    template <typename TOther>
        requires(std::is_convertible_v<TOther *, T *> && !std::is_const_v<TOther>)
    guard(const synchronized<TOther> &other) noexcept;

    // Unique guard is unique, after the assigment we would have two unique guards
    guard &operator=(const guard<T> &other) = delete;

    // Conversion move assignment operator from guard
    template <typename TOther>
        requires std::is_convertible_v<TOther *, T *>
    guard &operator=(guard<TOther> &&other) noexcept;

    // No conversion upgrade (immutable to mutable) move assignment.
    template <typename TOther>
        requires std::is_convertible_v<TOther *, T *>
    guard &operator=(guard<const TOther> &&other) noexcept = delete;

    guard &operator=(guard<T> &&other) noexcept;

    // No upgrade (immutable to mutable) move assignment.
    guard &operator=(guard<const T> &&other) noexcept = delete;

    // Conversion assignment operator from synchronized
    template <typename TOther>
        requires std::is_convertible_v<TOther *, T *>
    guard &operator=(const synchronized<TOther> &other) noexcept;

    ~guard();

    // Equality of guards, not the underlying objects --> similar to smart pointers
    [[nodiscard]] bool operator==(const guard &other) const noexcept;
    [[nodiscard]] bool operator!=(const guard &other) const noexcept;

    // Arrow operator
    [[nodiscard]] T *operator->() const;

    // Dereference operator
    [[nodiscard]] T &operator*() const;

    // Cast to T reference
    [[nodiscard]] explicit operator T &() const noexcept;

    // Cast to T pointer
    [[nodiscard]] explicit operator T *() const noexcept;

  private:
    template <typename TOther>
        requires(!std::is_const_v<TOther>)
    friend class synchronized;

    template <typename TOther>
    friend class guard;

    template <typename TOther>
        requires(std::is_convertible_v<TOther *, T *> && !std::is_const_v<TOther>)
    guard(ref<TOther> protected_instance, ref<shared_recursive_mutex> mutex) noexcept;

    void lock() noexcept;
    void unlock() noexcept;

    // track the protected instance via a ref to detect the destruction of the synchronized instance
    ref<T> protected_instance_;
    ref<shared_recursive_mutex> mutex_;
};

// The underlying lock is a shared lock for const access, so a separate implementation is needed.
template <typename T>
class guard<const T>
{
  public:
    using value_t = const T;

    class blindfold
    {
      public:
        explicit blindfold(guard &guard) :
            original_guard_(&guard),
            unlocked_guard_(std::move([](auto &&sent) {
                sent.unlock();
                return std::forward<decltype(sent)>(sent);
            }(std::move(guard))))
        {
        }

        blindfold(const blindfold &other) = delete;
        blindfold(blindfold &&other) :
            original_guard_(other.original_guard_),
            unlocked_guard_(std::move(other.unlocked_guard_))
        {
            other.original_guard_ = nullptr;
        }

        blindfold &operator=(const blindfold &other) = delete;
        blindfold &operator=(blindfold &&other)
        {
            if (this == &other)
            {
                return *this;
            }

            unblind_original_guard();

            original_guard_ = other.original_guard_;
            unlocked_guard_ = std::move(other.unlocked_guard_);
            other.original_guard_ = nullptr;

            return *this;
        }

        ~blindfold()
        {
            unblind_original_guard();
        }

      private:
        void unblind_original_guard()
        {
            if (original_guard_ != nullptr)
            {
                *original_guard_ = std::move(unlocked_guard_);
                original_guard_->lock();
                original_guard_ = nullptr;
            }
        }

        guard *original_guard_;
        guard unlocked_guard_;
    };

    // Conversion copy constructor
    template <typename TOther>
        requires std::is_convertible_v<TOther *, const T *>
    guard(const guard<const TOther> &other) noexcept;

    // No conversion downgrade (mutable to immutable) copy constructor.
    template <typename TOther>
        requires std::is_convertible_v<TOther *, const T *>
    guard(const guard<TOther> &other) noexcept = delete;

    // The conversion copy constructor does not cover the copy constructor, so we need to implement it explicitly
    guard(const guard<const T> &other) noexcept;

    // No downgrade (mutable to immutable) copy constructor.
    guard(const guard<T> &other) noexcept = delete;

    // Conversion move constructor
    template <typename TOther>
        requires std::is_convertible_v<TOther *, const T *>
    guard(guard<const TOther> &&other) noexcept;

    // No conversion downgrade (mutable to immutable) move constructor.
    template <typename TOther>
        requires std::is_convertible_v<TOther *, const T *>
    guard(guard<TOther> &&other) noexcept = delete;

    // The conversion move constructor does not cover the move constructor, so we need to implement it explicitly
    guard(guard<const T> &&other) noexcept;

    // No downgrade (mutable to immutable) move constructor.
    guard(guard<T> &&other) noexcept = delete;

    // Conversion copy construction from synchronized
    template <typename TOther>
        requires(std::is_convertible_v<TOther *, const T *> && !std::is_const_v<TOther>)
    guard(const synchronized<TOther> &other) noexcept;

    // Conversion copy assignment operator from guard
    template <typename TOther>
        requires std::is_convertible_v<TOther *, const T *>
    guard &operator=(const guard<const TOther> &other);

    // No conversion downgrade (mutable to immutable) copy assignment.
    template <typename TOther>
        requires std::is_convertible_v<TOther *, const T *>
    guard &operator=(const guard<TOther> &other) = delete;

    guard &operator=(const guard<const T> &other);

    // No downgrade (mutable to immutable) copy assignment.
    guard &operator=(const guard<T> &other) = delete;

    // Conversion move assignment operator from guard
    template <typename TOther>
        requires std::is_convertible_v<TOther *, const T *>
    guard &operator=(guard<const TOther> &&other) noexcept;

    // No conversion downgrade (mutable to immutable) move assignment.
    template <typename TOther>
        requires std::is_convertible_v<TOther *, const T *>
    guard &operator=(guard<TOther> &&other) noexcept = delete;

    guard &operator=(guard<const T> &&other) noexcept;

    // No downgrade (mutable to immutable) move assignment.
    guard &operator=(guard<T> &&other) noexcept = delete;

    // Conversion assignment operator from synchronized
    template <typename TOther>
        requires std::is_convertible_v<TOther *, const T *>
    guard &operator=(const synchronized<TOther> &other) noexcept;

    ~guard();

    // Equality of guards, not the underlying objects --> similar to smart pointers
    [[nodiscard]] bool operator==(const guard &other) const noexcept;

    [[nodiscard]] bool operator!=(const guard &other) const noexcept;

    // Arrow operator
    [[nodiscard]] const T *operator->() const;

    // Dereference operator
    [[nodiscard]] const T &operator*() const;

    // Cast to T reference
    [[nodiscard]] explicit operator const T &() const noexcept;

    // Cast to T pointer
    [[nodiscard]] explicit operator const T *() const noexcept;

  private:
    template <typename TOther>
        requires(!std::is_const_v<TOther>)
    friend class synchronized;

    template <typename TOther>
    friend class guard;

    template <typename TOther>
        requires(std::is_convertible_v<TOther *, T *> && !std::is_const_v<TOther>)
    guard(ref<const TOther> protected_instance, ref<shared_recursive_mutex> mutex) noexcept;

    void lock() noexcept;
    void unlock() noexcept;

    // track the protected instance via a ref to detect the destruction of the synchronized instance
    ref<const T> protected_instance_;
    ref<shared_recursive_mutex> mutex_;
};

}  // namespace saam

#include <saam/detail/guard.ipp>
