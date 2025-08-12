// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/detail/counted_ref/var.hpp>
#include <saam/panic.hpp>

namespace saam
{

template <class T>
class enable_ref_from_this
{
  public:
    enable_ref_from_this() = default;

    // Copy/move constructors only change the internal variable, but have no affect on the reference counting.
    enable_ref_from_this(const enable_ref_from_this &other) {};
    enable_ref_from_this(enable_ref_from_this &&other) noexcept {};

    // Copy/move assignment only change the internal variable, but have no affect on the reference counting.
    enable_ref_from_this &operator=(const enable_ref_from_this &other)
    {
        return *this;
    };
    enable_ref_from_this &operator=(enable_ref_from_this &&other) noexcept
    {
        return *this;
    };

    ~enable_ref_from_this() = default;

    // Mutable borrow
    [[nodiscard]] ref<T> borrow_from_this()
    {
        assert_that(manager_, "enable_ref_from_this: no var available");
        return {*manager_};
    }

    // Immutable borrow
    [[nodiscard]] ref<const T> borrow_from_this() const
    {
        assert_that(manager_, "enable_ref_from_this: no var available");
        return {*manager_};
    }

    void smart_variable(var<T> *manager)
    {
        manager_ = manager;
    }

  private:
    // Does not store a smart reference, it would make the var non destroyable.
    // Instead, it knows its owner var, so it can create a reference any time
    // without having one.
    var<T> *manager_ = nullptr;
};

}  // namespace saam
