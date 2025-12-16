// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/detail/basic_enable_ref_from_this.hpp>
#include <saam/detail/basic_ref.hpp>
#include <saam/detail/basic_var.hpp>

#include <atomic>
#include <cassert>
#include <limits>
#include <typeinfo>

namespace saam
{

// The user must define this function to handle the dangling reference situation.
// After this function, dangling reference(s) exist in the process, so the memory is possibly going to be corrupted soon.
// Therefore, after returning from this function, the process will be aborted.
void dangling_reference_panic(const std::type_info &var_type, void *var_instance, std::size_t num_dangling_references) noexcept;

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
            // "this" gets always the same reference counter as "other", so the count that "other" loses, gains "this"
            // -> no modification on the counter needed
            other.borrow_manager_ = nullptr;
        }

        ref_base &operator=(const ref_base &other)
        {
            if (this == &other)
            {
                return *this;
            }

            const bool change_in_borrow_manager = borrow_manager_ != other.borrow_manager_;
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

            const bool change_in_borrow_manager = borrow_manager_ != other.borrow_manager_;
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
            if (!is_managed())
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
        const auto prev_value = counter_--;
        // Reference count cannot go below zero, so the previous value must be greater than zero
        assert(prev_value > 0);
    }

    void verify_dangling_references(const std::type_info &var_type, void *var_instance) const noexcept
    {
        std::size_t prev_value = 0;
        const bool destroyed_with_active_references =
            !counter_.compare_exchange_strong(prev_value, std::numeric_limits<std::size_t>::max());
        if (destroyed_with_active_references)
        {
            const auto num_dangling_references = counter_.load();
            dangling_reference_panic(var_type, var_instance, num_dangling_references);
            abort();
        }
    }

  private:
    // maxint the counter was closed, no more borrows are allowed
    // 0 counter means that the instance is not borrowed
    // counter > 0 means that the instance is borrowed
    mutable std::atomic<std::size_t> counter_ = 0;
};

}  // namespace saam
