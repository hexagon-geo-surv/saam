// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/panic.hpp>
#include <saam/detail/tracked_ref/var.hpp>

#include <type_traits>

namespace saam
{

template <class T>
class enable_ref_from_this
{
  public:
    enable_ref_from_this() = default;

    // The var of the other instance is not relevant for the new instance, so do not take the var over
    enable_ref_from_this(const enable_ref_from_this &other) {};
    enable_ref_from_this(enable_ref_from_this &&other) noexcept {};

    // Reassigning the object does not change the var
    enable_ref_from_this &operator=(const enable_ref_from_this &other) {};
    enable_ref_from_this &operator=(enable_ref_from_this &&other) noexcept {};

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
    var<T> *manager_ = nullptr;
};

}  // namespace saam
