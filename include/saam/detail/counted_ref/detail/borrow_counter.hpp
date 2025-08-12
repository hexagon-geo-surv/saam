// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/panic.hpp>

#include <atomic>
#include <limits>

namespace saam
{

class borrow_counter
{
  public:
    borrow_counter() = default;

    // reference counters are not copied/moved, each var counts its own references
    borrow_counter(const borrow_counter &other) = delete;
    borrow_counter(borrow_counter &&other) noexcept = delete;
    borrow_counter &operator=(const borrow_counter &other) = delete;
    borrow_counter &operator=(borrow_counter &&other) noexcept = delete;

    bool is_borrowed() const
    {
        return counter_ != 0;
    }

    void allocate() const
    {
        counter_++;
    }

    void release() const
    {
        auto prev_value = counter_--;
        assert_that(prev_value > 0, "corrupted reference count");
    }

    std::size_t count() const
    {
        return counter_;
    }

    bool close_counting() const
    {
        std::size_t prev_value = 0;
        const bool closing_suceeded = counter_.compare_exchange_strong(prev_value, std::numeric_limits<std::size_t>::max());
        return closing_suceeded;
    }

  private:
    // maxint the counter was closed, no more borrows are allowed
    // 0 counter means that the instance is not borrowed
    // counter > 0 means that the instance is borrowed

    mutable std::atomic<std::size_t> counter_ = 0;
};

}  // namespace saam
