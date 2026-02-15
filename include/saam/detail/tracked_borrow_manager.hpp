// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/detail/borrow_manager_traits.hpp>

#include <cassert>
#include <functional>
#include <mutex>
#include <stacktrace>

namespace saam
{

// The user must define this function to handle the dangling reference situation.
// After this function, dangling reference(s) exist in the process, so the memory is possibly going to be corrupted soon.
// This function is called for each dangling reference detected.
// Therefore, after returning from this function, the process will be aborted.
using dangling_reference_panic_t = std::function<void(const std::type_info &var_type,
                                                      void *var_instance,
                                                      const std::stacktrace &var_destruction_stack,
                                                      std::size_t dangling_ref_index,
                                                      const std::stacktrace &dangling_ref_creation_stack)>;
inline dangling_reference_panic_t dangling_reference_panic;

class tracked_borrow_manager
{
  public:
    class ref_base
    {
      public:
        [[nodiscard]] bool is_managed() const
        {
            return borrow_manager_ != nullptr;
        }

        [[nodiscard]] tracked_borrow_manager *borrow_manager() const noexcept
        {
            return borrow_manager_;
        }

      protected:
        ref_base() = default;

        ref_base(tracked_borrow_manager *borrow_manager) :
            borrow_manager_(borrow_manager)
        {
            register_self();
        }

        ref_base(const ref_base &other) :
            borrow_manager_(other.borrow_manager_)
        {
            register_self();
        }

        ref_base(ref_base &&other) noexcept :
            borrow_manager_(other.borrow_manager_)
        {
            register_self();

            other.unregister_self();
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

        void register_self()
        {
            if (!is_managed())
            {
                return;
            }

            borrow_manager_->register_ref(*this);

            // The linked_ref is now attached to a var, let's capture the stacktrace of this moment
            stacktrace_ = std::stacktrace::current();
        }

        void unregister_self()
        {
            if (!is_managed())
            {
                return;
            }

            borrow_manager_->unregister_ref(*this);
            borrow_manager_ = nullptr;

            // The linked_ref is detached from any var, so the stacktrace is not relevant anymore
            stacktrace_ = std::stacktrace();
        }

      private:
        friend class tracked_borrow_manager;

        ref_base *next_ = nullptr;
        std::stacktrace stacktrace_;
        tracked_borrow_manager *borrow_manager_ = nullptr;
    };

    // reference management is copied/moved, each var manages its own references
    // regardless if it was assigned a another T instance
    tracked_borrow_manager(const tracked_borrow_manager &other) = delete;
    tracked_borrow_manager(tracked_borrow_manager &&other) noexcept = delete;
    tracked_borrow_manager &operator=(const tracked_borrow_manager &other) = delete;
    tracked_borrow_manager &operator=(tracked_borrow_manager &&other) noexcept = delete;
    ~tracked_borrow_manager() = default;

    void verify_dangling_references(const std::type_info &var_type, void *var_instance) const noexcept
    {
        std::lock_guard guard(mutex_);

        const bool destroyed_with_active_references = ref_chain_root_ != nullptr;
        if (destroyed_with_active_references)
        {
            std::size_t dangling_ref_index = 0;
            std::stacktrace var_destruction_stack = std::stacktrace::current();
            for (auto *current_link = ref_chain_root_; current_link != nullptr; current_link = current_link->next_)
            {
                const auto &ref_stacktrace = current_link->stacktrace_;
                if (dangling_reference_panic)
                {
                    dangling_reference_panic(var_type, var_instance, var_destruction_stack, dangling_ref_index, ref_stacktrace);
                }
                dangling_ref_index++;
            }

            abort();
        }
    }

  private:
    tracked_borrow_manager() = default;

    void register_ref(ref_base &ref)
    {
        std::lock_guard guard(mutex_);
        // Attach the new link to the beginning of the chain - we saved a walk to the end of the chain
        // Moreover, it is likely that new refs will die earlier than old ones
        ref.next_ = ref_chain_root_;
        ref_chain_root_ = &ref;
    }

    void unregister_ref(ref_base &linked_ref_to_detach)
    {
        std::lock_guard guard(mutex_);

        auto *previous_link_ptr = get_previous_ptr_in_chain(linked_ref_to_detach);
        *previous_link_ptr = (*previous_link_ptr)->next_;
    }

    ref_base **get_previous_ptr_in_chain(ref_base &ref_to_detach)
    {
        auto *previous_link_ptr = &ref_chain_root_;
        while (true)
        {
            ref_base *current_link = *previous_link_ptr;
            // Sanity check - the linked_ref to detach must be in the chain
            assert(current_link != nullptr);

            const bool link_to_detach_found = current_link == &ref_to_detach;
            if (link_to_detach_found)
            {
                return previous_link_ptr;
            }

            previous_link_ptr = &current_link->next_;
        }
    }

    template <typename TOther, borrow_manager TOtherBorrowManager>
    friend class var;

    mutable std::mutex mutex_;
    ref_base *ref_chain_root_ = nullptr;
};

}  // namespace saam
