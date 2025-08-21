// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/panic.hpp>
#include <saam/safe_ref.hpp>
#include <saam/sentinel.hpp>
#include <saam/synchronized.hpp>

#include <any>
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
    explicit condition(const synchronized<T> &synchronized)
        : protected_instance_(synchronized.protected_instance_.borrow())
    {
    }

    enum class wait_result : std::uint8_t
    {
        criteria_met,
        timeout
    };

    template <typename T, typename Clock = std::chrono::system_clock, typename Duration = std::chrono::milliseconds>
    wait_result wait(
        sentinel<T> &sentinel,
        const std::function<bool(const typename std::remove_reference_t<decltype(sentinel)>::value_t &)> &exit_criteria,
        std::optional<std::variant<std::chrono::milliseconds, std::chrono::time_point<Clock, Duration>>> maybe_timeout = std::nullopt)
    {
        assert_that(sentinel.protected_instance_ == *std::any_cast<saam::ref<std::remove_cv_t<T>>>(&protected_instance_),
                    "condition is not related to sentinel");

        auto waiting_predicate = [&sentinel, &exit_criteria]() { return exit_criteria(*sentinel); };

        if (!maybe_timeout.has_value())
        {
            condition_variable_.wait(sentinel.lock_, waiting_predicate);
            return wait_result::criteria_met;
        }

        auto &timeout = maybe_timeout.value();
        if (std::holds_alternative<std::chrono::milliseconds>(timeout))
        {
            const bool criteria_met =
                condition_variable_.wait_for(sentinel.lock_, std::get<std::chrono::milliseconds>(timeout), waiting_predicate);
            return criteria_met ? wait_result::criteria_met : wait_result::timeout;
        }

        const bool criteria_met =
            condition_variable_.wait_until(sentinel.lock_, std::get<std::chrono::time_point<Clock, Duration>>(timeout), waiting_predicate);
        return criteria_met ? wait_result::criteria_met : wait_result::timeout;
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
    std::condition_variable_any condition_variable_;
};

}  // namespace saam
