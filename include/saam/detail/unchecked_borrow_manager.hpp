// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/detail/basic_enable_ref_from_this.hpp>
#include <saam/detail/basic_ref.hpp>
#include <saam/detail/basic_var.hpp>
#include <saam/panic.hpp>

#include <typeinfo>

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
        ref_base() = default;

        ref_base(const ref_base &other) = default;

        // The conversion move constructor does not cover the move constructor, so we need to implement it explicitly
        ref_base(ref_base &&other) noexcept
            : borrow_manager_(other.borrow_manager_)
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

        [[nodiscard]] unchecked_borrow_manager *borrow_manager() const noexcept
        {
            return borrow_manager_;
        }

        // If the underlying raw pointer is managed reference (refrence counted)
        [[nodiscard]] bool is_managed() const noexcept
        {
            return false;
        }

        ref_base(unchecked_borrow_manager &borrow_counter)
            : borrow_manager_(&borrow_counter)
        {
        }

        unchecked_borrow_manager *borrow_manager_ = nullptr;
    };

    unchecked_borrow_manager() = default;

    // reference counters are not copied/moved, each var counts its own references
    unchecked_borrow_manager(const unchecked_borrow_manager &other) = delete;
    unchecked_borrow_manager(unchecked_borrow_manager &&other) noexcept = delete;
    unchecked_borrow_manager &operator=(const unchecked_borrow_manager &other) = delete;
    unchecked_borrow_manager &operator=(unchecked_borrow_manager &&other) noexcept = delete;

    void verify_dangling_references(const std::type_info &type) const noexcept
    {
    }
};

}  // namespace saam
