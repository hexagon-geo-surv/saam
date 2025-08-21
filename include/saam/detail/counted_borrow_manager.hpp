// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/detail/basic_enable_ref_from_this.hpp>
#include <saam/detail/basic_ref.hpp>
#include <saam/detail/basic_var.hpp>
#include <saam/panic.hpp>

#include <atomic>
#include <limits>
#include <sstream>
#include <typeinfo>

namespace saam
{

class counted_borrow_manager
{
  public:
    // For the manager to be able to manage the reference, the reference must derived from this base class.
    // This manager specific base class adds everything to a reference that is needed for its management.
    class ref_base
    {
      public:
        ref_base() = default;

        ref_base(counted_borrow_manager &borrow_manager)
            : borrow_manager_(&borrow_manager)
        {
            register_self();
        }

        ref_base(const ref_base &other)
            : borrow_manager_(other.borrow_manager_)
        {
            register_self();
        }

        ref_base(ref_base &&other) noexcept
            : borrow_manager_(other.borrow_manager_)
        {
            // "this" gets always the same reference counter as "other", so the count that "other" looses, gains "this"
            // -> no modification on the counter needed
            other.borrow_manager_ = nullptr;
        }

        ref_base &operator=(const ref_base &other)
        {
            if (this == &other)
            {
                return *this;
            }

            const bool change_in_borrow_manager = borrow_manager_ == other.borrow_manager_;
            if (change_in_borrow_manager)
            {
                unregister_self();

                borrow_manager_ = other.borrow_manager_;
                register_self();
            }

            return *this;
        }

        ref_base &operator=(ref_base &&other) noexcept
        {
            if (this == &other)
            {
                return *this;
            }

            const bool change_in_borrow_manager = borrow_manager_ == other.borrow_manager_;
            if (change_in_borrow_manager)
            {
                unregister_self();

                borrow_manager_ = other.borrow_manager_;
                register_self();
            }

            other.unregister_self();

            return *this;
        }

        ~ref_base()
        {
            unregister_self();
        }

        [[nodiscard]] counted_borrow_manager *borrow_manager() const noexcept
        {
            return borrow_manager_;
        }

        // If the underlying raw pointer is managed reference (refrence counted)
        [[nodiscard]] bool is_managed() const noexcept
        {
            return borrow_manager_ != nullptr;
        }

        void register_self() noexcept
        {
            if (!is_managed())
            {
                return;
            }

            borrow_manager_->register_reference();
        }

        void unregister_self() noexcept
        {
            if (global_panic_handler.is_panic_active() || !is_managed())
            {
                return;
            }

            borrow_manager_->unregister_reference();
            borrow_manager_ = nullptr;
        }

        counted_borrow_manager *borrow_manager_ = nullptr;
    };

    counted_borrow_manager() = default;

    // reference counters are not copied/moved, each var counts its own references
    counted_borrow_manager(const counted_borrow_manager &other) = delete;
    counted_borrow_manager(counted_borrow_manager &&other) noexcept = delete;
    counted_borrow_manager &operator=(const counted_borrow_manager &other) = delete;
    counted_borrow_manager &operator=(counted_borrow_manager &&other) noexcept = delete;

    void register_reference() const
    {
        counter_++;
    }

    void unregister_reference() const
    {
        auto prev_value = counter_--;
        assert_that(prev_value > 0, "corrupted reference count");
    }

    void verify_dangling_references(const std::type_info &type) const noexcept
    {
        const bool destroying_with_active_references = !close_counting();
        if (destroying_with_active_references)
        {
            std::ostringstream panic_message;
            panic_message << "Borrow checked variable of type <" << type.name() << "> destroyed with " << counter_
                          << " active reference(s).\n";
            global_panic_handler.trigger_panic(panic_message.str());
        }
    }

  private:
    bool close_counting() const
    {
        std::size_t prev_value = 0;
        const bool closing_suceeded = counter_.compare_exchange_strong(prev_value, std::numeric_limits<std::size_t>::max());
        return closing_suceeded;
    }

    // maxint the counter was closed, no more borrows are allowed
    // 0 counter means that the instance is not borrowed
    // counter > 0 means that the instance is borrowed

    mutable std::atomic<std::size_t> counter_ = 0;
};

}  // namespace saam
