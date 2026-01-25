// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/synchronized.hpp>

namespace saam
{

template <typename T>
    requires(!std::is_const_v<T>)
synchronized<T>::synchronized() = default;

template <typename T>
    requires(!std::is_const_v<T>)
template <typename... Args>
synchronized<T>::synchronized(std::in_place_t, Args &&...args) :
    protected_instance_(std::forward<Args>(args)...)
{
}

template <typename T>
    requires(!std::is_const_v<T>)
synchronized<T>::synchronized(const T &instance) :
    protected_instance_(instance)
{
}

template <typename T>
    requires(!std::is_const_v<T>)
synchronized<T>::synchronized(T &&instance) :
    protected_instance_(std::move(instance))
{
}

template <typename T>
    requires(!std::is_const_v<T>)
synchronized<T>::synchronized(const synchronized &other) :
    protected_instance_(*other.commence())
{
    // Just take the other instance, synchronizedes of "this" and "other" are independent
}

template <typename T>
    requires(!std::is_const_v<T>)
synchronized<T>::synchronized(synchronized &&other) noexcept :
    protected_instance_(std::move(*other.commence_mut()))
{
    // Just take the other instance, synchronizedes of "this" and "other" are independent
}
template <typename T>
    requires(!std::is_const_v<T>)
synchronized<T> &synchronized<T>::operator=(const synchronized<T> &other)
{
    if (this == &other)
    {
        return *this;
    }

    // std::scoped_lock lock(protector_mutex_, other.protector_mutex_);

    // Just take the other instance, synchronizedes of "this" and "other" are independent
    protected_instance_ = other.protected_instance_;

    return *this;
}

template <typename T>
    requires(!std::is_const_v<T>)
synchronized<T> &synchronized<T>::operator=(synchronized<T> &&other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    // std::scoped_lock lock(protector_mutex_, other.protector_mutex_);

    // Just take the other instance, mutexes of "this" and "other" are independent
    protected_instance_ = std::move(other.protected_instance_);

    return *this;
}

template <typename T>
    requires(!std::is_const_v<T>)
synchronized<T>::~synchronized() = default;

template <typename T>
    requires(!std::is_const_v<T>)
template <typename TOther>
synchronized<T> &synchronized<T>::use_mutex_of(ref<synchronized<TOther>> other)
{
    active_mutex_ = std::move(other->mutex_);
    return *this;
}

template <typename T>
    requires(!std::is_const_v<T>)
guard<T> synchronized<T>::commence_mut() const
{
    return guard<T>(*this);
}

template <typename T>
    requires(!std::is_const_v<T>)
guard<const T> synchronized<T>::commence() const
{
    return guard<const T>(*this);
}

template <typename T>
    requires(!std::is_const_v<T>)
template <typename... Args>
synchronized<T> &synchronized<T>::emplace(Args &&...args)
{
    *commence_mut() = T(std::forward<Args>(args)...);
    return *this;
}

template <typename T>
    requires(!std::is_const_v<T>)
[[nodiscard]] guard<T> synchronized<T>::operator->() const noexcept
{
    return guard<T>(*this);
}

template <typename T>
    requires(!std::is_const_v<T>)
synchronized<T> &synchronized<T>::operator=(const T &instance) noexcept
{
    *commence_mut() = instance;
    return *this;
}

template <typename T>
    requires(!std::is_const_v<T>)
synchronized<T> &synchronized<T>::operator=(T &&instance) noexcept
{
    *commence_mut() = std::move(instance);
    return *this;
}

//
// Condition class implementation
//

template <typename T>
    requires(!std::is_const_v<T>)
synchronized<T>::condition::condition(const synchronized &synched, std::function<bool(const T &)> fulfillment_criteria) :
    protected_instance_(synched.protected_instance_.borrow()),
    fulfillment_criteria_(std::move(fulfillment_criteria))
{
}

template <typename T>
    requires(!std::is_const_v<T>)
template <typename TClock, typename TDuration>
typename synchronized<T>::condition::wait_result synchronized<T>::condition::wait(
    guard<T> &guard, std::optional<std::variant<std::chrono::milliseconds, std::chrono::time_point<TClock, TDuration>>> maybe_timeout)
{
    assert(!guard.mutex_.is_moved_from());
    // Ensure that the condition is related to the guard
    assert(guard.protected_instance_ == protected_instance_);

    auto internal_mutex_lock{guard.mutex_->acquire_internal_mutex()};

    // Can't go sleep with holding the lock count, otherwise others cannot change the state of the synchronized instance
    (void)guard.mutex_->unregister_unique_count();
    // Do not notify yet the threads that are waiting to acquire a guard.
    // Delay it until the waiting predicate is called the first time.

    const auto waiting_predicate = [&]() {
        guard.mutex_->register_unique_count(internal_mutex_lock);
        const bool criteria_met = fulfillment_criteria_(*guard);
        if (!criteria_met)
        {
            auto no_lock_counts = guard.mutex_->unregister_unique_count();
            if (no_lock_counts)
            {
                // Even though the internal mutex is still locked here, this is the last chance for the notification
                // before going to sleep.
                guard.mutex_->notify_mutex_free_condition(true);
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
                internal_mutex_lock, std::get<std::chrono::time_point<TClock, TDuration>>(timeout), waiting_predicate);
            result = criteria_met ? wait_result::criteria_met : wait_result::timeout;
        }
    }

    return result;
}

template <typename T>
    requires(!std::is_const_v<T>)
template <typename TClock, typename TDuration>
typename synchronized<T>::condition::wait_result synchronized<T>::condition::wait(
    guard<const T> &guard, std::optional<std::variant<std::chrono::milliseconds, std::chrono::time_point<TClock, TDuration>>> maybe_timeout)
{
    assert(!guard.mutex_.is_moved_from());
    // Ensure that the condition is related to the guard
    assert(guard.protected_instance_ == protected_instance_);

    auto internal_mutex_lock{guard.mutex_->acquire_internal_mutex()};

    // Can't go sleep with holding the lock count, otherwise others cannot change the state of the synchronized instance
    (void)guard.mutex_->unregister_shared_count();
    // Do not notify yet the threads that are waiting to acquire a guard.
    // Delay it until the waiting predicate is called the first time.

    const auto waiting_predicate = [&]() {
        guard.mutex_->register_shared_count(internal_mutex_lock);
        const bool criteria_met = fulfillment_criteria_(*guard);
        if (!criteria_met)
        {
            auto no_lock_counts = guard.mutex_->unregister_shared_count();
            if (no_lock_counts)
            {
                // Even though the internal mutex is still locked here, this is the last chance for the notification
                // before going to sleep.
                guard.mutex_->notify_mutex_free_condition(true);
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
                internal_mutex_lock, std::get<std::chrono::time_point<TClock, TDuration>>(timeout), waiting_predicate);
            result = criteria_met ? wait_result::criteria_met : wait_result::timeout;
        }
    }

    return result;
}

template <typename T>
    requires(!std::is_const_v<T>)
void synchronized<T>::condition::notify_one()
{
    condition_variable_.notify_one();
}

template <typename T>
    requires(!std::is_const_v<T>)
void synchronized<T>::condition::notify_all()
{
    condition_variable_.notify_all();
}

}  // namespace saam
