// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/detail/counted_ref/detail/borrow_counter.hpp>
#include <saam/panic.hpp>

namespace saam
{

class counted_ref
{
  public:
    counted_ref() = default;

    counted_ref(const counted_ref &other)
        : borrow_counter_(other.borrow_counter_)
    {
        register_at_counter();
    }

    // The conversion move constructor does not cover the move constructor, so we need to implement it explicitly
    counted_ref(counted_ref &&other) noexcept
        : borrow_counter_(other.borrow_counter_)
    {
        // "this" gets always the same reference counter as "other", so the count that "other" looses, gains "this"
        // -> no modification on the counter needed
        other.borrow_counter_ = nullptr;
    }

    counted_ref &operator=(const counted_ref &other)
    {
        unregister_at_counter();

        borrow_counter_ = other.borrow_counter_;
        register_at_counter();

        return *this;
    }

    counted_ref &operator=(counted_ref &&other) noexcept
    {
        unregister_at_counter();

        // Do no allocate a new borrow count for "this", but steal the borrow count from "other"
        borrow_counter_ = other.borrow_counter_;
        other.borrow_counter_ = nullptr;

        return *this;
    }

    ~counted_ref()
    {
        unregister_at_counter();
    }

    // If the underlying raw pointer is managed reference (refrence counted)
    [[nodiscard]] bool is_managed() const noexcept
    {
        return borrow_counter_ != nullptr;
    }

    counted_ref(borrow_counter &borrow_counter)
        : borrow_counter_(&borrow_counter)
    {
        register_at_counter();
    }

    void register_at_counter() noexcept
    {
        if (!is_managed())
        {
            return;
        }

        borrow_counter_->allocate();
    }

    void unregister_at_counter() noexcept
    {
        if (global_panic_handler.is_panic_active() || !is_managed())
        {
            return;
        }

        borrow_counter_->release();
        borrow_counter_ = nullptr;
    }

    borrow_counter *borrow_counter_ = nullptr;
};

}  // namespace saam
