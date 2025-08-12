// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/detail/unchecked_ref/ref.hpp>

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
    var() = default;

    template <typename... Args>
    explicit var(std::in_place_t, Args &&...args)
        : instance_(std::forward<Args>(args)...)
    {
    }

    explicit var(const T &instance)
        : instance_(instance)
    {
    }

    explicit var(T &&instance)
        : instance_(std::move(instance))
    {
    }

    // No conversion copy constructor, because of the slicing problem
    var(const var &other)
        : var(other.instance_)
    {
        // Just take the other instance, reference management of "this" and "other" are independent
    }

    // No conversion move constructor, because of the slicing problem
    var(var &&other) noexcept
        : var(std::move(other.instance_))
    {
        // Just take the other instance, reference management of "this" and "other" are independent
    }

    // No conversion copy assignment, because of the slicing problem
    var &operator=(const var &other)
    {
        if (this == &other)
        {
            return *this;
        }

        // Just take the other instance, reference management of "this" and "other" are independent
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
        instance_ = std::move(other.instance_);
        return *this;
    }

    ~var() = default;

    // Borrowing is a const operation, because borrowing just provides access to the underlying object - does not change the manager
    [[nodiscard]] ref<T> borrow() const noexcept
    {
        return ref<T>(const_cast<var *>(this)->instance_);
    }

    // No direct casting to raw reference is allowed
    [[nodiscard]] explicit operator T &() const = delete;

  private:
    template <typename TVar>
    friend class var;

    template <typename TRef>
    friend class ref;

    T instance_;
};

template <typename T>
template <typename TVar>
    requires std::is_convertible_v<TVar *, T *>
ref<T>::ref(const var<TVar> &other) noexcept
    : ref(const_cast<var<TVar> &>(other).instance_)
{
}

template <typename T>
template <typename TVar>
    requires std::is_convertible_v<TVar *, T *>
ref<T> &ref<T>::operator=(const var<TVar> &other) noexcept
{
    instance_ = &(const_cast<TVar &>(other.instance_));
    return *this;
}

}  // namespace saam
