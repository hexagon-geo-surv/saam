// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/safe_ref.hpp>
#include <saam/sentinel.hpp>
#include <saam/shared_recursive_mutex.hpp>

#include <chrono>
#include <condition_variable>
#include <functional>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>

namespace saam
{

// Synchronized owns an instance of T and provides a thread-safe way to access it.
// The lifetime of T is bound to the lifetime of the synchronized.
template <typename T>
    requires(!std::is_const_v<T>)  // Only mutable types need synchronization
class synchronized
{
  public:
    class condition
    {
      public:
        condition(const synchronized &synched, std::function<bool(const T &)> fulfillment_criteria);

        enum class wait_result : std::uint8_t
        {
            criteria_met,
            timeout
        };

        template <typename TClock = std::chrono::system_clock, typename TDuration = std::chrono::milliseconds>
        wait_result wait(sentinel<T> &sentinel,
                         std::optional<std::variant<std::chrono::milliseconds, std::chrono::time_point<TClock, TDuration>>> maybe_timeout =
                             std::nullopt);

        template <typename TClock = std::chrono::system_clock, typename TDuration = std::chrono::milliseconds>
        wait_result wait(sentinel<const T> &sentinel,
                         std::optional<std::variant<std::chrono::milliseconds, std::chrono::time_point<TClock, TDuration>>> maybe_timeout =
                             std::nullopt);

        void notify_one();
        void notify_all();

      private:
        saam::ref<T> protected_instance_;
        std::condition_variable condition_variable_;
        std::function<bool(const T &)> fulfillment_criteria_;
    };

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

    // Use a mutex from another synchronized. This way the the same mutex protects the instances wrapped by of both synchronized
    // Useful, when the base class has a synchronized and the derived also has a synchronized and we want to use the mutex from the base
    // class.
    template <typename TOther>
    synchronized &use_mutex_of(ref<synchronized<TOther>> other);

    // Mutable lock
    [[nodiscard]] sentinel<T> lock_mut() const;

    // Immutable lock
    [[nodiscard]] sentinel<const T> lock() const;

  private:
    template <typename TOther>
        requires(!std::is_const_v<TOther>)
    friend class synchronized;

    template <typename TOther>
    friend class sentinel;

    var<shared_recursive_mutex> mutex_;
    ref<shared_recursive_mutex> active_mutex_{mutex_};

    // When a synchronized instance is released in a locked state, the outstanding locks contain invalid reference.
    // This case shall trigger a panic.
    var<T> protected_instance_;
};

}  // namespace saam

#include <saam/detail/synchronized.ipp>
