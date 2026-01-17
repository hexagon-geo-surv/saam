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
    protected_instance_(*other.lock())
{
    // Just take the other instance, synchronizedes of "this" and "other" are independent
}

template <typename T>
    requires(!std::is_const_v<T>)
synchronized<T>::synchronized(synchronized &&other) noexcept :
    protected_instance_(std::move(*other.lock_mut()))
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
sentinel<T> synchronized<T>::lock_mut() const
{
    return sentinel<T>(*this);
}

template <typename T>
    requires(!std::is_const_v<T>)
sentinel<const T> synchronized<T>::lock() const
{
    return sentinel<const T>(*this);
}

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
    sentinel<T> &sentinel, std::optional<std::variant<std::chrono::milliseconds, std::chrono::time_point<TClock, TDuration>>> maybe_timeout)
{
    assert(!sentinel.mutex_.is_moved_from());
    // Ensure that the condition is related to the sentinel
    assert(sentinel.protected_instance_ == protected_instance_);

    auto internal_mutex_lock{sentinel.mutex_->acquire_internal_mutex()};

    // Can't go sleep with holding the lock count, otherwise others cannot change the state of the synchronized instance
    (void)sentinel.mutex_->unregister_unique_count();
    // Do not notify yet the threads that are waiting to acquire a senrinel.
    // Delay it until the waiting predicate is called the first time.

    const auto waiting_predicate = [&]() {
        sentinel.mutex_->register_unique_count(internal_mutex_lock);
        const bool criteria_met = fulfillment_criteria_(*sentinel);
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
    sentinel<const T> &sentinel, std::optional<std::variant<std::chrono::milliseconds, std::chrono::time_point<TClock, TDuration>>> maybe_timeout)
{
    assert(!sentinel.mutex_.is_moved_from());
    // Ensure that the condition is related to the sentinel
    assert(sentinel.protected_instance_ == protected_instance_);

    auto internal_mutex_lock{sentinel.mutex_->acquire_internal_mutex()};

    // Can't go sleep with holding the lock count, otherwise others cannot change the state of the synchronized instance
    (void)sentinel.mutex_->unregister_shared_count();
    // Do not notify yet the threads that are waiting to acquire a senrinel.
    // Delay it until the waiting predicate is called the first time.

    const auto waiting_predicate = [&]() {
        sentinel.mutex_->register_shared_count(internal_mutex_lock);
        const bool criteria_met = fulfillment_criteria_(*sentinel);
        if (!criteria_met)
        {
            auto no_lock_counts = sentinel.mutex_->unregister_shared_count();
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
