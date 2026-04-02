// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/guard.hpp>
#include <saam/safe_ref.hpp>

#include <mutex>
#include <optional>
#include <shared_mutex>
#include <tuple>
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
    using mutex_t = std::shared_mutex;
    using data_type_t = T;

    synchronized();

    template <typename... Args>
    explicit synchronized(std::in_place_t, Args &&...args);

    explicit synchronized(const T &instance);

    explicit synchronized(T &&instance);

    // No conversion copy constructor, because of the slicing of T - only the base class would be copied
    synchronized(const synchronized &other);

    // No conversion move constructor, because of the slicing of T - only the base class would be copied
    synchronized(synchronized &&other) noexcept;

    // No conversion copy assignment, because of the slicing of T - only the base class would be copied
    synchronized &operator=(const synchronized &other);

    // No conversion move assignment, because of the slicing of T - only the base class would be copied
    synchronized &operator=(synchronized &&other) noexcept;

    // No race condition is possible during destruction
    // If another thread is in the class then it must have a smart reference (ref instance),
    // so the smart owner (var instance) does not let wrapped the synchronized deleted destructed
    ~synchronized();

    // All borrowings are const operations, because borrowing just provides access to the underlying object - does not change the managed
    // wrapper.

    // Mutable unique lock
    [[nodiscard]] guard<T> commence_mut() const;

    // Immutable shared lock
    [[nodiscard]] guard<const T> commence() const;

    // In-place re-construction of the underlying type - internally uses the mutable guard
    template <typename... Args>
    synchronized &emplace(Args &&...args);

    // During the access to the underlying object, there must be a temporary smart reference. The lifetime of the temporary smart reference
    // starts before the operator-> is called and ends well after the call is completed. Without this, we use the underlying object without
    // administrating it in the borrow manager and a parallel destruction of the var would NOT consider this access for the final reference
    // check. The first operator-> provides a temporary smart reference. Then the call into the underlying object is done via the smart
    // reference's operator->. The two operators-> are collapsed into one operator-> by the C++ compiler.
    [[nodiscard]] guard<T> operator->() const noexcept;

    // Assignment from underlying type - internally uses the mutable guard
    synchronized &operator=(const T &instance) noexcept;
    synchronized &operator=(T &&instance) noexcept;

    // No direct casting to raw reference is allowed
    [[nodiscard]] operator T &() const = delete;

  private:
    template <typename TOther>
        requires(!std::is_const_v<TOther>)
    friend class synchronized;

    template <typename TOther>
    friend class guard;

    mutable mutex_t mutex_;

    // When a synchronized instance is released in a locked state, the outstanding locks contain invalid reference.
    // This case shall trigger a panic.
    var<T> protected_instance_;

    template <typename... TOther>
    friend auto commence_all(synchronized<std::remove_const_t<TOther>> &...syncs);
};

template <typename... T>
auto commence_all(synchronized<std::remove_const_t<T>> &...syncs)
{
    while (true)
    {
        {
            // Try acquiring all locks into a tuple without blocking.
            bool all_guards_acquired = true;
            auto maybe_guards = std::make_tuple([&](const auto &sync) -> std::optional<guard<T>> {
                if constexpr (std::is_const_v<T>)
                {
                    std::shared_lock lock(sync.mutex_, std::try_to_lock);
                    if (lock.owns_lock())
                    {
                        return guard<T>(ref<T>(sync.protected_instance_), std::move(lock));
                    }
                }
                else
                {
                    std::unique_lock lock(sync.mutex_, std::try_to_lock);
                    if (lock.owns_lock())
                    {
                        return guard<T>(ref<T>(sync.protected_instance_), std::move(lock));
                    }
                }

                all_guards_acquired = false;
                return std::nullopt;
            }(syncs)...);

            if (all_guards_acquired)
            {
                // Strip the std::optional and keep only the guards
                return std::apply([](auto &&...optionals) { return std::make_tuple(std::move(optionals.value())...); }, maybe_guards);
            }

            // The guards are not complete (failed to acquire them all), release them all here
        }

        // Let's probe the locks and wait for the ones, which are not available. This avoids a busy wait retry.
        // Try to acquire the locks one by one with blocking to see if it is time to try to acquire them all again.
        // Acquire them only one at a time (do not keep the guard) - so no race condition can happen.
        // After a probing round, let's try to acquire them all again.
        (..., [](const auto &sync) {
            if constexpr (std::is_const_v<T>)
            {
                sync.mutex_.lock_shared();
                sync.mutex_.unlock_shared();
            }
            else
            {
                sync.mutex_.lock();
                sync.mutex_.unlock();
            }
        }(syncs));
    }
}

}  // namespace saam

#include <saam/detail/synchronized.ipp>
