// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/detail/borrow_manager_traits.hpp>

namespace saam
{

class unchecked_borrow_manager
{
  public:
    // For the manager to be able to manage the reference, the reference must derived from this base class.
    // This manager specific base class adds everything to a reference that is needed for its management.
    class ref_base
    {
      public:
        [[nodiscard]] unchecked_borrow_manager *borrow_manager() const noexcept
        {
            return borrow_manager_;
        }

        // If the underlying raw pointer is managed reference (refrence counted)
        [[nodiscard]] bool is_managed() const noexcept
        {
            return false;
        }

      protected:
        ref_base() = default;

        ref_base(const ref_base &other) = default;

        // The conversion move constructor does not cover the move constructor, so we need to implement it explicitly
        ref_base(ref_base &&other) noexcept :
            borrow_manager_(other.borrow_manager_)
        {
            // "this" gets always the same reference counter as "other", so the count that "other" looses, gains "this"
            // -> no modification on the counter needed
            other.borrow_manager_ = nullptr;
        }

        ref_base &operator=(const ref_base &other) = default;

        ref_base &operator=(ref_base &&other) noexcept
        {
            if (this == &other)
            {
                return *this;
            }

            // Do no allocate a new borrow count for "this", but steal the borrow count from "other"
            borrow_manager_ = other.borrow_manager_;
            other.borrow_manager_ = nullptr;

            return *this;
        }

        ~ref_base() = default;

        ref_base(unchecked_borrow_manager &borrow_counter) :
            borrow_manager_(&borrow_counter)
        {
        }

        unchecked_borrow_manager *borrow_manager_ = nullptr;
    };

    // reference management is copied/moved, each var manages its own references
    // regardless if it was assigned a another T instance
    unchecked_borrow_manager(const unchecked_borrow_manager &other) = delete;
    unchecked_borrow_manager(unchecked_borrow_manager &&other) noexcept = delete;
    unchecked_borrow_manager &operator=(const unchecked_borrow_manager &other) = delete;
    unchecked_borrow_manager &operator=(unchecked_borrow_manager &&other) noexcept = delete;

  private:
    unchecked_borrow_manager() = default;

    template <typename TOther, borrow_manager TOtherBorrowManager>
    friend class basic_var;
};

}  // namespace saam
