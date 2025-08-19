// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/detail/basic_ref.hpp>
#include <saam/detail/borrow_manager.hpp>
#include <saam/detail/constructor_destructor_traits.hpp>

#include <type_traits>
#include <utility>

namespace saam
{

template <class T, borrow_manager TBorrowManager>
class basic_enable_ref_from_this;

template <class T, borrow_manager TBorrowManager>
class basic_var
{
  public:
    basic_var()
    {
        configure_enable_ref_from_this();
        call_post_constructor();
    }

    template <typename... Args>
    explicit basic_var(std::in_place_t, Args &&...args)
        : instance_(std::forward<Args>(args)...)
    {
        configure_enable_ref_from_this();
        call_post_constructor();
    }

    explicit basic_var(const T &instance)
        : instance_(instance)
    {
        configure_enable_ref_from_this();
        call_post_constructor();
    }

    explicit basic_var(T &&instance)
        : instance_(std::move(instance))
    {
        configure_enable_ref_from_this();
        call_post_constructor();
    }

    // No conversion copy constructor, because of the slicing of T - only the base class would be copied
    basic_var(const basic_var &other)
        : basic_var(other.instance_)
    {
        // Just take the other instance, reference counters of "this" and "other" are independent
    }

    // No conversion move constructor, because of the slicing of T - only the base class would be copied
    basic_var(basic_var &&other) noexcept
        : basic_var(std::move(other.instance_))
    {
        // Just take the other instance, reference counters of "this" and "other" are independent
    }

    // No conversion copy assignment, because of the slicing of T - only the base class would be copied
    basic_var &operator=(const basic_var &other) noexcept
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
    basic_var &operator=(basic_var &&other) noexcept
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

    ~basic_var()
    {
        // Pre-destructor offers the possibility to do cleanup before the owned object is destroyed.
        // It is a great place to revoke callbacks that contain self references.
        call_pre_destructor();

        // Before destroying the owned object, we need to check if there are any active references
        // The desctruction of the owned object cannot be done before the reference check,
        // because it would allow a data race between a thread inside the owned object and the destructor of the owned object.
        borrow_manager_.verify_dangling_references(typeid(T));
    }

    // Borrowing is a const operation, because borrowing just provides access to the underlying object - does not change the manager
    [[nodiscard]] basic_ref<T, TBorrowManager> borrow() const noexcept
    {
        return basic_ref<T, TBorrowManager>(const_cast<basic_var *>(this)->instance_, borrow_manager_);
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
        if constexpr (std::is_base_of_v<basic_enable_ref_from_this<T, TBorrowManager>, T>)
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

    template <typename TOther, borrow_manager TOtherBorrowManager>
    friend class basic_var;

    template <typename TOther, borrow_manager TOtherBorrowManager>
    friend class basic_ref;

    mutable TBorrowManager borrow_manager_;
    T instance_;
};

template <typename T, borrow_manager TBorrowManager>
template <typename TOther>
    requires std::is_convertible_v<TOther *, T *>
basic_ref<T, TBorrowManager>::basic_ref(const basic_var<TOther, TBorrowManager> &other) noexcept
    : basic_ref(const_cast<basic_var<TOther, TBorrowManager> &>(other).instance_,
                const_cast<basic_var<TOther, TBorrowManager> &>(other).borrow_manager_)
{
}

template <typename T, borrow_manager TBorrowManager>
template <typename TOther>
    requires std::is_convertible_v<TOther *, T *>
basic_ref<T, TBorrowManager> &basic_ref<T, TBorrowManager>::operator=(const basic_var<TOther, TBorrowManager> &other) noexcept
{
    operator=(basic_ref(const_cast<basic_var<TOther, TBorrowManager> &>(other).instance_,
                        const_cast<basic_var<TOther, TBorrowManager> &>(other).borrow_manager_));

    return *this;
}

}  // namespace saam
