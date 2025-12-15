// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/detail/basic_var.hpp>

#include <cassert>

namespace saam
{

template <class T, borrow_manager TRefManager>
class basic_enable_ref_from_this
{
  public:
    basic_enable_ref_from_this() = default;

    // Copy/move constructors only change the internal variable, but have no affect on the reference counting.
    basic_enable_ref_from_this(const basic_enable_ref_from_this &other) {};
    basic_enable_ref_from_this(basic_enable_ref_from_this &&other) noexcept {};

    // Copy/move assignment only change the internal variable, but have no affect on the reference counting.
    basic_enable_ref_from_this &operator=(const basic_enable_ref_from_this & /*other*/)
    {
        return *this;
    }

    basic_enable_ref_from_this &operator=(basic_enable_ref_from_this && /*other*/) noexcept
    {
        return *this;
    }

    ~basic_enable_ref_from_this() = default;

    // Mutable borrow
    [[nodiscard]] basic_ref<T, TRefManager> borrow_from_this()
    {
        assert(manager_);
        return {*manager_};
    }

    // Immutable borrow
    [[nodiscard]] basic_ref<const T, TRefManager> borrow_from_this() const
    {
        assert(manager_);
        return {*manager_};
    }

    void smart_variable(basic_var<T, TRefManager> *manager)
    {
        manager_ = manager;
    }

  private:
    // Does not store a smart reference, it would make the var non destroyable.
    // Instead, it knows its owner var, so it can create a reference any time
    // without having one.
    basic_var<T, TRefManager> *manager_ = nullptr;
};

}  // namespace saam
