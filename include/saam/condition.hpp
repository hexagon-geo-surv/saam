// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/safe_ref.hpp>
#include <saam/sentinel.hpp>
#include <saam/synchronized.hpp>

#include <any>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <optional>
#include <type_traits>
#include <variant>

namespace saam
{

// Represents a condition that can be waited on
class condition
{
  public:
    template <typename T>
    explicit condition(const synchronized<T> &synchronized) :
        protected_instance_(synchronized.protected_instance_.borrow())
    {
    }

    enum class wait_result : std::uint8_t
    {
        criteria_met,
        timeout
    };

    template <typename T, typename Clock = std::chrono::system_clock, typename Duration = std::chrono::milliseconds>
        requires(!std::is_const_v<T>)
    wait_result wait(
        sentinel<T> &sentinel,
        const std::function<bool(const typename std::remove_reference_t<decltype(sentinel)>::value_t &)> &exit_criteria,
        std::optional<std::variant<std::chrono::milliseconds, std::chrono::time_point<Clock, Duration>>> maybe_timeout = std::nullopt)
    {
        assert(!sentinel.mutex_.is_moved_from());
        // Ensure that the condition is related to the sentinel
        assert(sentinel.protected_instance_ == *std::any_cast<saam::ref<std::remove_cv_t<T>>>(&protected_instance_));

        auto internal_mutex_lock{sentinel.mutex_->acquire_internal_mutex()};

        // Can't go sleep with holding the lock count, otherwise others cannot change the state of the synchronized instance
        (void)sentinel.mutex_->unregister_unique_count();
        // Do not notify yet the threads that are waiting to acquire a senrinel.
        // Delay it until the waiting predicate is called the first time.

        const auto waiting_predicate = [&internal_mutex_lock, &sentinel, &exit_criteria]() {
            sentinel.mutex_->register_unique_count(internal_mutex_lock);
            const bool criteria_met = exit_criteria(*sentinel);
            if (!criteria_met)
            {
                auto no_lock_counts = sentinel.mutex_->unregister_unique_count();
                if (no_lock_counts)
                {
                    // Even though the internal mutex is still locked here, this is the last change for the notification
                    // before going to sleep.
                    sentinel.mutex_->notify_mutex_free_condition(true);
                }
            }
            return criteria_met;
        };

        wait_result result;

        if (!maybe_timeout.has_value())
        {
            condition_variable_.wait(internal_mutex_lock, waiting_predicate);
            result = wait_result::criteria_met;
        }
        else
        {
            auto &timeout = maybe_timeout.value();
            if (std::holds_alternative<std::chrono::milliseconds>(timeout))
            {
                const bool criteria_met =
                    condition_variable_.wait_for(internal_mutex_lock, std::get<std::chrono::milliseconds>(timeout), waiting_predicate);
                result = criteria_met ? wait_result::criteria_met : wait_result::timeout;
            }
            else
            {
                const bool criteria_met = condition_variable_.wait_until(
                    internal_mutex_lock, std::get<std::chrono::time_point<Clock, Duration>>(timeout), waiting_predicate);
                result = criteria_met ? wait_result::criteria_met : wait_result::timeout;
            }
        }

        return result;
    }

    enum class notification_scope : std::uint8_t
    {
        one_waiter,
        all_waiters,
    };

    void notify(notification_scope scope = notification_scope::all_waiters)
    {
        if (scope == notification_scope::one_waiter)
        {
            condition_variable_.notify_one();
        }
        else
        {
            condition_variable_.notify_all();
        }
    }

  private:
    std::any protected_instance_;  // Hold a type-erased reference to the synchronized instance
    std::condition_variable condition_variable_;
};

}  // namespace saam
