// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/panic.hpp>

#include <mutex>
#include <sstream>
#include <stacktrace>

namespace saam
{

class tracked_borrow_manager
{
  public:
    class ref_base
    {
      public:
        ref_base() = default;

        ref_base(tracked_borrow_manager &borrow_manager)
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

        ref_base &operator=(ref_base &&other)
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

        [[nodiscard]] bool is_managed() const
        {
            return borrow_manager_ != nullptr;
        }

        [[nodiscard]] tracked_borrow_manager *borrow_manager() const noexcept
        {
            return borrow_manager_;
        }

        void register_self()
        {
            if (!is_managed())
            {
                return;
            }

            borrow_manager_->register_ref(*this);

            // The linked_ref is now attached to a var, let's capture the stacktrace of this moment
            if (borrow_manager_->stack_tracking_enabled())
            {
                stacktrace_ = std::stacktrace::current();
            }
        }

        void unregister_self()
        {
            if (global_panic_handler.is_panic_active() || !is_managed())
            {
                return;
            }

            borrow_manager_->unregister_ref(*this);
            borrow_manager_ = nullptr;

            // The linked_ref is detached from any var, so the stacktrace is not relevant anymore
            stacktrace_ = std::stacktrace();
        }

        ref_base *next_ = nullptr;
        std::stacktrace stacktrace_;
        tracked_borrow_manager *borrow_manager_ = nullptr;
    };

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

    void verify_dangling_references(const std::type_info &type) const noexcept
    {
        auto active_ref_stacktraces = collect_active_ref_stacktraces();
        const bool destroyed_with_active_references = !active_ref_stacktraces.empty();
        if (destroyed_with_active_references)
        {
            std::ostringstream panic_message;
            panic_message << "Borrow checked variable of type <" << type.name() << "> destroyed at \n"
                          << std::stacktrace::current() << "\n\n";
            panic_message << "still has active references. Active reference(s) created at: \n";

            for (const auto &stacktrace : active_ref_stacktraces)
            {
                if (!stacktrace.empty())
                {
                    panic_message << stacktrace << '\n';
                }
                else
                {
                    panic_message << "No stack trace available\n";
                }

                panic_message << "-------------------------------\n";
            }
            global_panic_handler.trigger_panic(panic_message.str());
        }
    }

    std::vector<std::stacktrace> collect_active_ref_stacktraces() const
    {
        std::lock_guard guard(mutex_);

        std::vector<std::stacktrace> stacktraces;
        for (auto current_link = ref_chain_root_; current_link != nullptr; current_link = current_link->next_)
        {
            stacktraces.push_back(current_link->stacktrace_);
        }

        return stacktraces;
    }

    bool stack_tracking_enabled() const
    {
        std::lock_guard guard(mutex_);
        return stack_tracking_enabled_;
    }

    tracked_borrow_manager &stack_tracking_enabled(bool enabled)
    {
        std::lock_guard guard(mutex_);
        stack_tracking_enabled_ = enabled;
        return *this;
    }

  private:
    ref_base **get_previous_ptr_in_chain(ref_base &ref_to_detach)
    {
        auto *previous_link_ptr = &ref_chain_root_;
        while (true)
        {
            ref_base *current_link = *previous_link_ptr;
            assert_that(current_link != nullptr, "linked_ref not found, ref chain is corrupted");

            const bool link_to_detach_found = current_link == &ref_to_detach;
            if (link_to_detach_found)
            {
                return previous_link_ptr;
            }

            previous_link_ptr = &current_link->next_;
        }
    }

    mutable std::mutex mutex_;
    bool stack_tracking_enabled_ = false;
    ref_base *ref_chain_root_ = nullptr;
};

}  // namespace saam
