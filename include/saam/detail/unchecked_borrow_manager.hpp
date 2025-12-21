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
        [[nodiscard]] static unchecked_borrow_manager *borrow_manager() noexcept
        {
            return nullptr;
        }

        // If the underlying raw pointer is managed reference (refrence counted)
        [[nodiscard]] static bool is_managed() noexcept
        {
            return false;
        }

      protected:
        ref_base() = default;

        ref_base(unchecked_borrow_manager *)
        {
        }

        ref_base(const ref_base &other) = default;
        ref_base(ref_base &&) noexcept = default;
        ref_base &operator=(const ref_base &other) = default;
        ref_base &operator=(ref_base &&) noexcept = default;
        ~ref_base() = default;
    };

    // reference management is copied/moved, each var manages its own references
    // regardless if it was assigned a another T instance
    unchecked_borrow_manager(const unchecked_borrow_manager &other) = delete;
    unchecked_borrow_manager(unchecked_borrow_manager &&other) noexcept = delete;
    unchecked_borrow_manager &operator=(const unchecked_borrow_manager &other) = delete;
    unchecked_borrow_manager &operator=(unchecked_borrow_manager &&other) noexcept = delete;
    ~unchecked_borrow_manager() = default;

  private:
    unchecked_borrow_manager() = default;

    template <typename TOther, borrow_manager TOtherBorrowManager>
    friend class basic_var;
};

}  // namespace saam
