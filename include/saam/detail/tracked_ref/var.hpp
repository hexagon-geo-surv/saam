// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/detail/constructor_destructor_traits.hpp>
#include <saam/detail/tracked_ref/detail/reference_chain.hpp>
#include <saam/detail/tracked_ref/ref.hpp>

#include <sstream>
#include <stacktrace>
#include <type_traits>
#include <utility>

namespace saam
{

template <class T>
class enable_ref_from_this;

template <class T>
class var
{
  public:
    var()
    {
        reference_chain_.stack_tracking_enabled(stack_tracking_enabled_for_type_);
        configure_enable_ref_from_this();
        call_post_constructor();
    }

    template <typename... Args>
    explicit var(std::in_place_t, Args &&...args)
        : instance_(std::forward<Args>(args)...)
    {
        reference_chain_.stack_tracking_enabled(stack_tracking_enabled_for_type_);
        configure_enable_ref_from_this();
        call_post_constructor();
    }

    explicit var(const T &instance)
        : instance_(instance)
    {
        reference_chain_.stack_tracking_enabled(stack_tracking_enabled_for_type_);
        configure_enable_ref_from_this();
        call_post_constructor();
    }

    explicit var(T &&instance)
        : instance_(std::move(instance))
    {
        reference_chain_.stack_tracking_enabled(stack_tracking_enabled_for_type_);
        configure_enable_ref_from_this();
        call_post_constructor();
    }

    // No conversion copy constructor, because of the slicing problem
    var(const var &other)
        : var(other.instance_)
    {
        // Just take the other instance, reference management of "this" and "other" are independent
        reference_chain_.stack_tracking_enabled(other.reference_chain_.stack_tracking_enabled());
    }

    // No conversion move constructor, because of the slicing problem
    var(var &&other) noexcept
        : var(std::move(other.instance_))
    {
        // Just take the other instance, reference management of "this" and "other" are independent
        reference_chain_.stack_tracking_enabled(other.reference_chain_.stack_tracking_enabled());
    }

    // No conversion copy assignment, because of the slicing problem
    var &operator=(const var &other)
    {
        if (this == &other)
        {
            return *this;
        }

        // Just take the other instance, reference management of "this" and "other" are independent
        // enable_ref_from_this support is managed by the instance itself via the enable_ref_from_this base class
        instance_ = other.instance_;
        return *this;
    }

    // No conversion move assignment, because of the slicing problem
    var &operator=(var &&other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }

        // Just take the other instance, reference management of "this" and "other" are independent
        // enable_ref_from_this support is managed by the instance itself via the enable_ref_from_this base class
        instance_ = std::move(other.instance_);
        return *this;
    }

    ~var()
    {
        // Pre-destructor offers the possibility to do cleanup before the owned object is destroyed.
        // It is a great place to revoke callbacks that contain self references.
        call_pre_destructor();

        // Before destroying the owned object, we need to check if there are any active references
        // The desctruction of the owned object cannot be done before the referenc check,
        // because it would allow a data race between a thread inside the owned object and the destructor of the owned object.

        auto active_ref_stacktraces = reference_chain_.collect_active_ref_stacktraces();
        const bool destroyed_with_active_references = !active_ref_stacktraces.empty();
        if (destroyed_with_active_references)
        {
            std::ostringstream panic_message;
            panic_message << "Borrow checked variable of type <" << typeid(T).name() << "> destroyed at \n"
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

    // Borrowing is a const operation, because borrowing just provides access to the underlying object - does not change the manager
    [[nodiscard]] ref<T> borrow() const noexcept
    {
        return ref<T>(const_cast<var *>(this)->instance_, reference_chain_);
    }

    // No direct casting to raw reference is allowed
    [[nodiscard]] explicit operator T &() const = delete;

    var &enable_instance_stack_tracking(bool enable)
    {
        reference_chain_.stack_tracking_enabled(enable);
        return *this;
    }

    static void enable_type_stack_tracking(bool enable)
    {
        stack_tracking_enabled_for_type_ = enable;
    }

  private:
    void call_pre_destructor()
    {
        if constexpr (has_pre_destructor<T>)
        {
            instance_.pre_destructor();
        }
    }

    void configure_enable_ref_from_this()
    {
        if constexpr (std::is_base_of_v<enable_ref_from_this<T>, T>)
        {
            instance_.smart_variable(this);
        }
    }

    void call_post_constructor()
    {
        if constexpr (has_post_constructor<T>)
        {
            instance_.post_constructor();
        }
    }

    template <typename TVar>
    friend class var;

    template <typename TRef>
    friend class ref;

    mutable reference_chain reference_chain_;
    T instance_;
    inline static bool stack_tracking_enabled_for_type_ = false;
};

template <typename T>
template <typename TVar>
    requires std::is_convertible_v<TVar *, T *>
ref<T>::ref(const var<TVar> &other) noexcept
    : ref(const_cast<var<TVar> &>(other).instance_, other.reference_chain_)
{
}

template <typename T>
template <typename TVar>
    requires std::is_convertible_v<TVar *, T *>
ref<T> &ref<T>::operator=(const var<TVar> &other) noexcept
{
    change_ref_chain(&other.reference_chain_);

    instance_ = &(const_cast<var<TVar> &>(other).instance_);

    return *this;
}

}  // namespace saam
