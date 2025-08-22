// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#include <saam/safe_ref.hpp>

#include <functional>
#include <memory>
#include <utility>

namespace saam
{

template <typename T>
class any_ptr
{
  private:
    std::function<T *()> accessor_;

    template <typename U>
    friend class any_ptr;

  public:
    any_ptr() = default;

    explicit any_ptr(std::function<T *()> accessor)
        : accessor_(std::move(accessor))
    {
    }

    template <typename U>
        requires std::is_convertible_v<U *, T *>
    explicit any_ptr(std::function<U *()> accessor)
        : accessor_(std::move(accessor))
    {
    }

    template <typename U>
        requires std::is_convertible_v<U *, T *>
    any_ptr(const any_ptr<U> &other)
        : accessor_(other.accessor_)
    {
    }

    template <typename U>
        requires std::is_convertible_v<U *, T *>
    any_ptr(any_ptr<U> &&other)
        : accessor_(std::move(other.accessor_))
    {
    }

    // Copy assignment copies the underlying lambda together with the captured variables
    template <typename U>
        requires std::is_convertible_v<U *, T *>
    any_ptr &operator=(const any_ptr<U> &other)
    {
        accessor_ = other.accessor_;
        return *this;
    }

    // Move assignment moves the underlying lambda together with the captured variables
    template <typename U>
        requires std::is_convertible_v<U *, T *>
    any_ptr &operator=(any_ptr<U> &&other)
    {
        accessor_ = std::move(other.accessor_);
        return *this;
    }

    T *operator->() const
    {
        return accessor_();
    }

    T &operator*() const
    {
        return *accessor_();
    }

    explicit operator bool() const
    {
        return static_cast<bool>(accessor_) && accessor_() != nullptr;
    }

    bool operator!() const
    {
        return !static_cast<bool>(*this);
    }

    void reset()
    {
        accessor_ = {};
    }
};

// Raw any_ptr factory function
template <typename T>
auto make_any_ptr(T &instance)
{
    return any_ptr<T>([&instance]() -> T * { return &instance; });
}

template <typename T>
auto make_any_ptr(T *ptr)
{
    return any_ptr<T>([ptr]() -> T * { return ptr; });
}

// Smart pointer any_ptr factory function
template <typename T>
auto make_any_ptr(std::shared_ptr<T> ptr)
{
    return any_ptr<T>([ptr = std::move(ptr)]() -> T * { return ptr.get(); });
}

// unique_ptr cannot be used, otherwise the std::function cannot be copied
template <typename T>
auto make_any_ptr(std::unique_ptr<T> ptr) = delete;

// Smart reference any_ptr factory function
template <typename T>
auto make_any_ptr(saam::var<T> &var)
{
    return any_ptr<T>([ref = var.borrow()]() -> T * { return &(*ref); });
}

template <typename T>
auto make_any_ptr(saam::ref<T> ref)
{
    return any_ptr<T>([ref = std::move(ref)]() -> T * { return &(*ref); });
}

}  // namespace saam
