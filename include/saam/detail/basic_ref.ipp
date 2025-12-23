// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#pragma once

#include <saam/detail/basic_ref.hpp>
#include <saam/detail/basic_var.hpp>

#include <type_traits>

namespace saam
{

template <typename T, borrow_manager TBorrowManager>
basic_ref<T, TBorrowManager>::basic_ref(T &instance) :
    basic_ref(instance, nullptr)
{
}

template <typename T, borrow_manager TBorrowManager>
basic_ref<T, TBorrowManager>::basic_ref(T &instance, TBorrowManager *borrow_manager) :
    TBorrowManager::ref_base(borrow_manager),
    instance_(&instance)
{
}

template <typename T, borrow_manager TBorrowManager>
template <typename TOther>
    requires std::is_convertible_v<TOther *, T *>
basic_ref<T, TBorrowManager>::basic_ref(const basic_ref<TOther, TBorrowManager> &other) :
    TBorrowManager::ref_base(other),
    instance_(other.instance_)
{
}

template <typename T, borrow_manager TBorrowManager>
basic_ref<T, TBorrowManager>::basic_ref(const basic_ref &other) :
    TBorrowManager::ref_base(other),
    instance_(other.instance_)
{
}

template <typename T, borrow_manager TBorrowManager>
template <typename TOther>
    requires std::is_convertible_v<TOther *, T *>
basic_ref<T, TBorrowManager>::basic_ref(basic_ref<TOther, TBorrowManager> &&other) noexcept :
    TBorrowManager::ref_base(std::move(other)),
    instance_(other.instance_)
{
    other.instance_ = nullptr;
}

template <typename T, borrow_manager TBorrowManager>
basic_ref<T, TBorrowManager>::basic_ref(basic_ref &&other) noexcept :
    TBorrowManager::ref_base(std::move(other)),
    instance_(other.instance_)
{
    other.instance_ = nullptr;
}

template <typename T, borrow_manager TBorrowManager>
template <typename TOther>
    requires std::is_convertible_v<TOther *, T *>
basic_ref<T, TBorrowManager>::basic_ref(const basic_var<TOther, TBorrowManager> &other) noexcept :
    basic_ref(const_cast<basic_var<TOther, TBorrowManager> &>(other).instance_,
              &const_cast<basic_var<TOther, TBorrowManager> &>(other).borrow_manager_)
{
}

template <typename T, borrow_manager TBorrowManager>
template <typename TOther>
    requires std::is_convertible_v<TOther *, T *>
basic_ref<T, TBorrowManager> &basic_ref<T, TBorrowManager>::operator=(const basic_ref<TOther, TBorrowManager> &other)
{
    instance_ = other.instance_;

    TBorrowManager::ref_base::operator=(other);

    return *this;
}

template <typename T, borrow_manager TBorrowManager>
basic_ref<T, TBorrowManager> &basic_ref<T, TBorrowManager>::operator=(const basic_ref &other)
{
    if (this == &other)
    {
        return *this;
    }

    instance_ = other.instance_;

    TBorrowManager::ref_base::operator=(other);

    return *this;
}

template <typename T, borrow_manager TBorrowManager>
template <typename TOther>
    requires std::is_convertible_v<TOther *, T *>
basic_ref<T, TBorrowManager> &basic_ref<T, TBorrowManager>::operator=(basic_ref<TOther, TBorrowManager> &&other) noexcept
{
    instance_ = other.instance_;
    other.instance_ = nullptr;

    TBorrowManager::ref_base::operator=(std::move(other));

    return *this;
}

template <typename T, borrow_manager TBorrowManager>
basic_ref<T, TBorrowManager> &basic_ref<T, TBorrowManager>::operator=(basic_ref &&other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    instance_ = other.instance_;
    other.instance_ = nullptr;

    TBorrowManager::ref_base::operator=(std::move(other));

    return *this;
}

template <typename T, borrow_manager TBorrowManager>
template <typename TOther>
    requires std::is_convertible_v<TOther *, T *>
basic_ref<T, TBorrowManager> &basic_ref<T, TBorrowManager>::operator=(const basic_var<TOther, TBorrowManager> &other) noexcept
{
    operator=(basic_ref(const_cast<basic_var<TOther, TBorrowManager> &>(other).instance_,
                        &const_cast<basic_var<TOther, TBorrowManager> &>(other).borrow_manager_));

    return *this;
}

template <typename T, borrow_manager TBorrowManager>
bool basic_ref<T, TBorrowManager>::operator==(const basic_ref &other) const noexcept
{
    return instance_ == other.instance_;
}

template <typename T, borrow_manager TBorrowManager>
bool basic_ref<T, TBorrowManager>::operator!=(const basic_ref &other) const noexcept
{
    return !(instance_ == other.instance_);
}

template <typename T, borrow_manager TBorrowManager>
T *basic_ref<T, TBorrowManager>::operator->() const noexcept
{
    // A reference is always bound to an object, so no check is needed - unless it is in a moved from state
    return instance_;
}

template <typename T, borrow_manager TBorrowManager>
T &basic_ref<T, TBorrowManager>::operator*() const & noexcept
{
    // A reference is always bound to an object, so no check is needed - unless it is in a moved from state
    return *instance_;
}

template <typename T, borrow_manager TBorrowManager>
[[nodiscard]] T &&basic_ref<T, TBorrowManager>::operator*() const && noexcept
{
    // A reference is always bound to an object, so no check is needed - unless it is in a moved from state
    return std::move(*instance_);
}

template <typename T, borrow_manager TBorrowManager>
basic_ref<T, TBorrowManager>::operator T &() const & noexcept
{
    // A reference is always bound to an object, so no check is needed - unless it is in a moved from state
    return *instance_;
}

template <typename T, borrow_manager TBorrowManager>
basic_ref<T, TBorrowManager>::operator T &&() const && noexcept
{
    // A reference is always bound to an object, so no check is needed - unless it is in a moved from state
    return std::move(*instance_);
}

template <typename T, borrow_manager TBorrowManager>
basic_ref<T, TBorrowManager>::operator T *() const noexcept
{
    // A reference is always bound to an object, so no check is needed - unless it is in a moved from state
    return instance_;
}

template <typename T, borrow_manager TBorrowManager>
template <typename TOther>
    requires std::is_base_of_v<T, TOther>
basic_ref<TOther, TBorrowManager> basic_ref<T, TBorrowManager>::static_down_cast() const
{
    // This is an explicit operation, so it is safe to cast
    return basic_ref<TOther, TBorrowManager>(static_cast<TOther &>(*instance_), TBorrowManager::ref_base::borrow_manager());
}

template <typename T, borrow_manager TBorrowManager>
template <typename TOther>
    requires std::is_base_of_v<T, TOther>
basic_ref<TOther, TBorrowManager> basic_ref<T, TBorrowManager>::dynamic_down_cast() const
{
    // This is an explicit operation, so it is safe to cast
    return basic_ref<TOther, TBorrowManager>(dynamic_cast<TOther &>(*instance_), TBorrowManager::ref_base::borrow_manager());
}

}  // namespace saam
