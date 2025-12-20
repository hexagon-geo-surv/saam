// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#include <saam/safe_ref.hpp>
#include <saam/synchronized.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <utility>

namespace saam::test
{
class base_a
{
  public:
    void post_constructor(saam::current_borrow_manager_t &borrow_manager)
    {
        borrow_manager_ = &borrow_manager;
        // It is possible from now on to create smart references to "this"
        auto smart_self_ref = saam::ref<base_a>(*this, *borrow_manager_);
    }

  private:
    saam::current_borrow_manager_t *borrow_manager_{nullptr};
    int num_{0};
};

class base_b
{
  public:
    void post_constructor(saam::current_borrow_manager_t &borrow_manager)
    {
        borrow_manager_ = &borrow_manager;
        // It is possible from now on to create smart references to "this"
        auto smart_self_ref = saam::ref<base_b>(*this, *borrow_manager_);
    }

  private:
    saam::current_borrow_manager_t *borrow_manager_{nullptr};
    double flnum_{0.0};
};

// class best_practice : public saam::enable_ref_from_this<best_practice>, private a, private b
class best_practice : private base_a, private base_b
{
    // Data members of the class are grouped into one or more struct(s)
    // Each of these aggregates will be protected by a mutex
    struct members
    {
        int data = 0;
        std::vector<std::string> data_collection;

        static members create(int data)
        {
            // Prepare the members; the object does not exist yet
            // If something is wrong with the creation, exit at this stage
            std::vector<std::string> data_collection(10, "Hello");

            // Push the created components into the struct
            return {.data = data, .data_collection = std::move(data_collection)};
        }
    };

    saam::current_borrow_manager_t *borrow_manager_{nullptr};
    // Smart mutex to synchronize member variables
    saam::synchronized<members> synced_m_;

  public:
    // Move the members under the control of the mutex
    // Return value optimization (RVO) may eliminate the cost of this function call
    best_practice(int data) :
        synced_m_(members::create(data))
    {
    }

    void post_constructor(saam::current_borrow_manager_t &borrow_manager)
    {
        borrow_manager_ = &borrow_manager;
        // Porpagate the post_constructor call to base classes
        base_a::post_constructor(borrow_manager);
        base_b::post_constructor(borrow_manager);
    }

    // The smart mutex is not movable (because the STL mutex is also not movable)
    // Move only the "members" from the other instance under the control of "this" smart mutex.
    best_practice(best_practice &&other) noexcept :
        synced_m_(
            std::move(other.synced_m_.lock_mut().rval_ref()))  // Locked "other" prevents modifications in "other" during the move operation
    {
    }

    auto get_data_comparator()
    {
        // Capturing the smart reference into the callback ensures a valid call destination.
        auto external_callback = [self = saam::ref<best_practice>(*this, *borrow_manager_)](int data_query) {
            return data_query == self->synced_m_.lock()->data;
        };
        return external_callback;
    }
};

TEST(best_practice_test, demo)
{
    constexpr int value = 5;
    saam::var<best_practice> bestprac(value);
    auto comparator = bestprac.borrow()->get_data_comparator();
    ASSERT_TRUE(comparator(value));
}

}  // namespace saam::test
