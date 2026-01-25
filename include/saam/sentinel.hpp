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
// The sentinel is expected to be a short living object, its lifetime is often bound to scope or it is a temporal variable. It is not
// supposed to be stored for a long time.
template <typename T>
class sentinel
{
  public:
    using value_t = T;

    class blindfold
    {
      public:
        explicit blindfold(sentinel &sentinel) :
            original_sentinel_(&sentinel),
            unlocked_sentinel_(std::move([](auto &&sent) {
                sent.unlock();
                return std::forward<decltype(sent)>(sent);
            }(std::move(sentinel))))
        {
        }

        blindfold(const blindfold &other) = delete;
        blindfold(blindfold &&other) :
            original_sentinel_(other.original_sentinel_),
            unlocked_sentinel_(std::move(other.unlocked_sentinel_))
        {
            other.original_sentinel_ = nullptr;
        }

        blindfold &operator=(const blindfold &other) = delete;
        blindfold &operator=(blindfold &&other)
        {
            if (this == &other)
            {
                return *this;
            }

            unblind_original_sentinel();

            original_sentinel_ = other.original_sentinel_;
            unlocked_sentinel_ = std::move(other.unlocked_sentinel_);
            other.original_sentinel_ = nullptr;

            return *this;
        }

        ~blindfold()
        {
            unblind_original_sentinel();
        }

      private:
        void unblind_original_sentinel()
        {
            if (original_sentinel_ != nullptr)
            {
                *original_sentinel_ = std::move(unlocked_sentinel_);
                original_sentinel_->lock();
                original_sentinel_ = nullptr;
            }
        }

        sentinel *original_sentinel_;
        sentinel unlocked_sentinel_;
    };

    // Unique sentinel is unique, after the copy we would have two unique sentinels, which is not unique anymore
    sentinel(const sentinel<T> &other) = delete;

    // Conversion move constructor
    template <typename TOther>
        requires std::is_convertible_v<TOther *, T *>
    sentinel(sentinel<TOther> &&other) noexcept;

    // No conversion upgrade (immutable to mutable) move constructor.
    template <typename TOther>
        requires std::is_convertible_v<TOther *, T *>
    sentinel(sentinel<const TOther> &&other) noexcept = delete;

    // The conversion move constructor does not cover the move constructor, so we need to implement it explicitly
    sentinel(sentinel<T> &&other) noexcept;

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
    sentinel &operator=(sentinel<TOther> &&other) noexcept;

    // No conversion upgrade (immutable to mutable) move assignment.
    template <typename TOther>
        requires std::is_convertible_v<TOther *, T *>
    sentinel &operator=(sentinel<const TOther> &&other) noexcept = delete;

    sentinel &operator=(sentinel<T> &&other) noexcept;

    // No upgrade (immutable to mutable) move assignment.
    sentinel &operator=(sentinel<const T> &&other) noexcept = delete;

    // Conversion assignment operator from synchronized
    template <typename TOther>
        requires std::is_convertible_v<TOther *, T *>
    sentinel &operator=(const synchronized<TOther> &other) noexcept;

    ~sentinel();

    // Equality of sentinels, not the underlying objects --> similar to smart pointers
    [[nodiscard]] bool operator==(const sentinel &other) const noexcept;
    [[nodiscard]] bool operator!=(const sentinel &other) const noexcept;

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
    friend class sentinel;

    void lock() noexcept;
    void unlock() noexcept;

    // track the protected instance via a ref to detect the destruction of the synchronized instance
    ref<T> protected_instance_;
    ref<shared_recursive_mutex> mutex_;
};

// The underlying lock is a shared lock for const access, so a separate implementation is needed.
template <typename T>
class sentinel<const T>
{
  public:
    using value_t = const T;

    class blindfold
    {
      public:
        explicit blindfold(sentinel &sentinel) :
            original_sentinel_(&sentinel),
            unlocked_sentinel_(std::move([](auto &&sent) {
                sent.unlock();
                return std::forward<decltype(sent)>(sent);
            }(std::move(sentinel))))
        {
        }

        blindfold(const blindfold &other) = delete;
        blindfold(blindfold &&other) :
            original_sentinel_(other.original_sentinel_),
            unlocked_sentinel_(std::move(other.unlocked_sentinel_))
        {
            other.original_sentinel_ = nullptr;
        }

        blindfold &operator=(const blindfold &other) = delete;
        blindfold &operator=(blindfold &&other)
        {
            if (this == &other)
            {
                return *this;
            }

            unblind_original_sentinel();

            original_sentinel_ = other.original_sentinel_;
            unlocked_sentinel_ = std::move(other.unlocked_sentinel_);
            other.original_sentinel_ = nullptr;

            return *this;
        }

        ~blindfold()
        {
            unblind_original_sentinel();
        }

      private:
        void unblind_original_sentinel()
        {
            if (original_sentinel_ != nullptr)
            {
                *original_sentinel_ = std::move(unlocked_sentinel_);
                original_sentinel_->lock();
                original_sentinel_ = nullptr;
            }
        }

        sentinel *original_sentinel_;
        sentinel unlocked_sentinel_;
    };

    // Conversion copy constructor
    template <typename TOther>
        requires std::is_convertible_v<TOther *, const T *>
    sentinel(const sentinel<const TOther> &other) noexcept;

    // No conversion downgrade (mutable to immutable) copy constructor.
    template <typename TOther>
        requires std::is_convertible_v<TOther *, const T *>
    sentinel(const sentinel<TOther> &other) noexcept = delete;

    // The conversion copy constructor does not cover the copy constructor, so we need to implement it explicitly
    sentinel(const sentinel<const T> &other) noexcept;

    // No downgrade (mutable to immutable) copy constructor.
    sentinel(const sentinel<T> &other) noexcept = delete;

    // Conversion move constructor
    template <typename TOther>
        requires std::is_convertible_v<TOther *, const T *>
    sentinel(sentinel<const TOther> &&other) noexcept;

    // No conversion downgrade (mutable to immutable) move constructor.
    template <typename TOther>
        requires std::is_convertible_v<TOther *, const T *>
    sentinel(sentinel<TOther> &&other) noexcept = delete;

    // The conversion move constructor does not cover the move constructor, so we need to implement it explicitly
    sentinel(sentinel<const T> &&other) noexcept;

    // No downgrade (mutable to immutable) move constructor.
    sentinel(sentinel<T> &&other) noexcept = delete;

    // Conversion copy construction from synchronized
    template <typename TOther>
        requires(std::is_convertible_v<TOther *, const T *> && !std::is_const_v<TOther>)
    sentinel(const synchronized<TOther> &other) noexcept;

    // Conversion copy assignment operator from sentinel
    template <typename TOther>
        requires std::is_convertible_v<TOther *, const T *>
    sentinel &operator=(const sentinel<const TOther> &other);

    // No conversion downgrade (mutable to immutable) copy assignment.
    template <typename TOther>
        requires std::is_convertible_v<TOther *, const T *>
    sentinel &operator=(const sentinel<TOther> &other) = delete;

    sentinel &operator=(const sentinel<const T> &other);

    // No downgrade (mutable to immutable) copy assignment.
    sentinel &operator=(const sentinel<T> &other) = delete;

    // Conversion move assignment operator from sentinel
    template <typename TOther>
        requires std::is_convertible_v<TOther *, const T *>
    sentinel &operator=(sentinel<const TOther> &&other) noexcept;

    // No conversion downgrade (mutable to immutable) move assignment.
    template <typename TOther>
        requires std::is_convertible_v<TOther *, const T *>
    sentinel &operator=(sentinel<TOther> &&other) noexcept = delete;

    sentinel &operator=(sentinel<const T> &&other) noexcept;

    // No downgrade (mutable to immutable) move assignment.
    sentinel &operator=(sentinel<T> &&other) noexcept = delete;

    // Conversion assignment operator from synchronized
    template <typename TOther>
        requires std::is_convertible_v<TOther *, const T *>
    sentinel &operator=(const synchronized<TOther> &other) noexcept;

    ~sentinel();

    // Equality of sentinels, not the underlying objects --> similar to smart pointers
    [[nodiscard]] bool operator==(const sentinel &other) const noexcept;

    [[nodiscard]] bool operator!=(const sentinel &other) const noexcept;

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
    friend class sentinel;

    void lock() noexcept;
    void unlock() noexcept;

    // track the protected instance via a ref to detect the destruction of the synchronized instance
    ref<const T> protected_instance_;
    ref<shared_recursive_mutex> mutex_;
};

}  // namespace saam

#include <saam/detail/sentinel.ipp>
