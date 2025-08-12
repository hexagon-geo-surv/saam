// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/detail/constructor_destructor_traits.hpp>
#include <saam/detail/counted_ref/detail/borrow_counter.hpp>
#include <saam/detail/counted_ref/ref.hpp>

#include <sstream>
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
        configure_enable_ref_from_this();
        call_post_constructor();
    }

    template <typename... Args>
    explicit var(std::in_place_t, Args &&...args)
        : instance_(std::forward<Args>(args)...)
    {
        configure_enable_ref_from_this();
        call_post_constructor();
    }

    explicit var(const T &instance)
        : instance_(instance)
    {
        configure_enable_ref_from_this();
        call_post_constructor();
    }

    explicit var(T &&instance)
        : instance_(std::move(instance))
    {
        configure_enable_ref_from_this();
        call_post_constructor();
    }

    // No conversion copy constructor, because of the slicing of T - only the base class would be copied
    var(const var &other)
        : var(other.instance_)
    {
        // Just take the other instance, reference counters of "this" and "other" are independent
    }

    // No conversion move constructor, because of the slicing of T - only the base class would be copied
    var(var &&other) noexcept
        : var(std::move(other.instance_))
    {
        // Just take the other instance, reference counters of "this" and "other" are independent
    }

    // No conversion copy assignment, because of the slicing of T - only the base class would be copied
    var &operator=(const var &other) noexcept
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

    // No conversion move assignment, because of the slicing of T - only the base class would be copied
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

        const bool destroying_with_active_references = !borrow_counter_.close_counting();
        if (destroying_with_active_references)
        {
            std::ostringstream panic_message;
            panic_message << "Borrow checked variable of type <" << typeid(T).name() << "> destroyed with " << borrow_counter_.count()
                          << " active reference(s).\n";
            global_panic_handler.trigger_panic(panic_message.str());
        }
    }

    // Borrowing is a const operation, because borrowing just provides access to the underlying object - does not change the manager
    [[nodiscard]] ref<T> borrow() const noexcept
    {
        return ref<T>(const_cast<var *>(this)->instance_, borrow_counter_);
    }

    // No direct casting to raw reference is allowed
    [[nodiscard]] operator T &() const = delete;

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

    mutable borrow_counter borrow_counter_;
    T instance_;
};

template <typename T>
template <typename TVar>
    requires std::is_convertible_v<TVar *, T *>
ref<T>::ref(const var<TVar> &other) noexcept
    : ref(const_cast<var<TVar> &>(other).instance_, const_cast<var<TVar> &>(other).borrow_counter_)
{
}

template <typename T>
template <typename TVar>
    requires std::is_convertible_v<TVar *, T *>
ref<T> &ref<T>::operator=(const var<TVar> &other) noexcept
{
    const bool references_already_the_var = borrow_counter_ == &other.borrow_counter_;
    if (references_already_the_var)
    {
        return *this;
    }

    unregister_at_counter();
    borrow_counter_ = &other.borrow_counter_;
    register_at_counter();
    instance_ = &(const_cast<TVar &>(other.instance_));

    return *this;
}

}  // namespace saam
