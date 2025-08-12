// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/panic.hpp>

#include <mutex>
#include <stacktrace>

namespace saam
{

class reference_chain;

class linked_ref
{
  public:
    linked_ref() = default;

    linked_ref(reference_chain *link_container)
    {
        register_self_at_ref_chain(link_container);
    }

    linked_ref(const linked_ref &other)
    {
        // The new instance is created from a different stack trace than the source
        register_self_at_ref_chain(other.reference_chain_);
    }

    linked_ref(linked_ref &&other)
    {
        // The new instance is created from a different stack trace than the source
        register_self_at_ref_chain(other.reference_chain_);

        other.unregister_self_at_ref_chain();
    }

    void change_ref_chain(reference_chain *other_reference_chain)
    {
        const bool no_change_in_reference_chain = reference_chain_ == other_reference_chain;
        if (no_change_in_reference_chain)
        {
            return;
        }

        unregister_self_at_ref_chain();
        register_self_at_ref_chain(other_reference_chain);
    }

    linked_ref &operator=(const linked_ref &other)
    {
        change_ref_chain(other.reference_chain_);
        return *this;
    }

    linked_ref &operator=(linked_ref &&other)
    {
        change_ref_chain(other.reference_chain_);
        other.unregister_self_at_ref_chain();

        return *this;
    }

    ~linked_ref()
    {
        unregister_self_at_ref_chain();
    }

    [[nodiscard]] bool is_managed() const
    {
        return reference_chain_ != nullptr;
    }

    void register_self_at_ref_chain(reference_chain *link_container);

    void unregister_self_at_ref_chain();

    linked_ref *next_ = nullptr;
    std::stacktrace stacktrace_;
    reference_chain *reference_chain_ = nullptr;
};

class reference_chain
{
  public:
    void register_ref(linked_ref &ref)
    {
        std::lock_guard guard(mutex_);
        // Attach the new link to the beginning of the chain - we saved a walk to the end of the chain
        // Moreover, it is likely that new refs will die earlier than old ones
        ref.next_ = ref_chain_root_;
        ref_chain_root_ = &ref;
    }

    void unregister_ref(linked_ref &linked_ref_to_detach)
    {
        std::lock_guard guard(mutex_);

        auto *previous_link_ptr = get_previous_ptr_in_chain(linked_ref_to_detach);
        *previous_link_ptr = (*previous_link_ptr)->next_;
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

    reference_chain &stack_tracking_enabled(bool enabled)
    {
        std::lock_guard guard(mutex_);
        stack_tracking_enabled_ = enabled;
        return *this;
    }

  private:
    linked_ref **get_previous_ptr_in_chain(linked_ref &ref_to_detach)
    {
        auto *previous_link_ptr = &ref_chain_root_;
        while (true)
        {
            linked_ref *current_link = *previous_link_ptr;
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
    linked_ref *ref_chain_root_ = nullptr;
};

inline void linked_ref::register_self_at_ref_chain(reference_chain *chain)
{
    reference_chain_ = chain;

    if (!is_managed())
    {
        return;
    }

    reference_chain_->register_ref(*this);

    // The linked_ref is now attached to a var, let's capture the stacktrace of this moment
    if (reference_chain_->stack_tracking_enabled())
    {
        stacktrace_ = std::stacktrace::current();
    }
}

inline void linked_ref::unregister_self_at_ref_chain()
{
    if (global_panic_handler.is_panic_active() || !is_managed())
    {
        return;
    }

    reference_chain_->unregister_ref(*this);
    reference_chain_ = nullptr;

    // The linked_ref is detached from any var, so the stacktrace is not relevant anymore
    stacktrace_ = std::stacktrace();
}

}  // namespace saam
