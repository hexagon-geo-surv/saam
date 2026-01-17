// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cassert>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace saam
{

// Works like std::shared_mutex, but allows recursive unique locking by the same thread.
class shared_recursive_mutex
{
  public:
    shared_recursive_mutex() = default;
    shared_recursive_mutex(const shared_recursive_mutex &) = delete;
    shared_recursive_mutex &operator=(const shared_recursive_mutex &) = delete;
    shared_recursive_mutex(shared_recursive_mutex &&) = delete;
    shared_recursive_mutex &operator=(shared_recursive_mutex &&) = delete;
    ~shared_recursive_mutex() = default;

    // Follows the std::shared_mutex interface. This is important for compatibility with std::shared_lock.
    void lock()
    {
        auto lock = acquire_internal_mutex();
        register_unique_count(lock);
    }

    bool try_lock()
    {
        auto lock = acquire_internal_mutex();
        return try_register_unique_count();
    }

    void unlock()
    {
        auto lock = acquire_internal_mutex();
        auto is_mutex_free = unregister_unique_count();
        lock.unlock();
        // Notify all, because there can be shared and other unique sentinels waiting
        notify_mutex_free_condition(true);
    }

    void lock_shared()
    {
        auto lock = acquire_internal_mutex();
        register_shared_count(lock);
    }

    bool try_lock_shared()
    {
        auto lock = acquire_internal_mutex();
        return try_register_shared_count();
    }

    void unlock_shared()
    {
        auto lock = acquire_internal_mutex();
        const auto is_mutex_free = unregister_shared_count();
        lock.unlock();
        // Notify only one, because shared sentinels are not waiting. They can just acquire the lock immediately.
        // There can be though multiple unique sentinels waiting, but only one of them can acquire the lock.
        notify_mutex_free_condition(false);
    }

  private:
    std::unique_lock<std::mutex> acquire_internal_mutex()
    {
        return std::unique_lock{internal_mutex_};
    }

    void register_unique_count(std::unique_lock<std::mutex> &lock)
    {
        const auto current_thread_id = std::this_thread::get_id();
        mutex_free_condition_.wait(lock, [&]() { return lock_is_free() || mutex_is_locked_unique_by_thread(current_thread_id); });
        unique_owner_thread_ = current_thread_id;
        --lock_count_;
    }

    bool try_register_unique_count()
    {
        const auto current_thread_id = std::this_thread::get_id();
        if (lock_is_free() || mutex_is_locked_unique_by_thread(current_thread_id))
        {
            unique_owner_thread_ = current_thread_id;
            --lock_count_;
            return true;
        }
        return false;
    }

    bool unregister_unique_count()
    {
        // Unique lock acquisition ensures that only the owner thread can unlock
        assert(mutex_is_locked_unique_by_thread(std::this_thread::get_id()));
        ++lock_count_;
        const bool lock_released = lock_is_free();
        return lock_released;
    }

    void register_shared_count(std::unique_lock<std::mutex> &lock)
    {
        mutex_free_condition_.wait(lock, [&]() { return lock_is_free() || mutex_is_locked_shared(); });
        ++lock_count_;
    }

    bool try_register_shared_count()
    {
        if (lock_is_free() || mutex_is_locked_shared())
        {
            ++lock_count_;
            return true;
        }
        return false;
    }

    bool unregister_shared_count()
    {
        assert(mutex_is_locked_shared());
        --lock_count_;
        const bool lock_released = lock_is_free();
        return lock_released;
    }

    void notify_mutex_free_condition(const bool notify_all)
    {
        if (notify_all)
        {
            mutex_free_condition_.notify_all();
        }
        else
        {
            mutex_free_condition_.notify_one();
        }
    }

    bool lock_is_free() const
    {
        return lock_count_ == 0;
    }

    bool mutex_is_locked_unique() const
    {
        return lock_count_ < 0;
    }

    bool mutex_is_locked_unique_by_thread(std::thread::id current_thread_id) const
    {
        return mutex_is_locked_unique() && (unique_owner_thread_ == current_thread_id);
    }

    bool mutex_is_locked_shared() const
    {
        return lock_count_ > 0;
    }

    // Condition needs access to the mutex internal functions
    template <typename T>
        requires(!std::is_const_v<T>)
    friend class synchronized;

    std::mutex internal_mutex_;
    std::thread::id unique_owner_thread_;
    // 0 unlocked
    // positive values: number of shared locks
    // negative values: number of recursive exclusive locks from the owner thread
    std::int64_t lock_count_{0};
    std::condition_variable mutex_free_condition_;
};

}  // namespace saam
