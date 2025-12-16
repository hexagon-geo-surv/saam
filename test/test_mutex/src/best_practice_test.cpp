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

class best_practice : public saam::enable_ref_from_this<best_practice>
{
    // Data members of the class are grouped into one or more struct(s)
    // Each of these aggregates will be protected by a mutex
    struct members
    {
        int data = 0;

        static members create(int data)
        {
            // Prepare the members; the object does not exist yet
            // If something is wrong with the creation, exit at this stage

            // Push the created components into the struct
            return {.data = data};
        }
    };

    // Smart mutex to synchronize member variables
    saam::synchronized<members> synced_m_;

  public:
    best_practice(int data)
        // Move the members under the control of the mutex
        // Return value optimization (RVO) may eliminate the cost of this function call
        : synced_m_(members::create(data))
    {
    }

    // The smart mutex is not movable (because the STL mutex is also not movable)
    // Move only the "members" from the other instance under the control of "this" smart mutex.
    best_practice(best_practice &&other)
        : synced_m_(std::move(*other.synced_m_.lock()))  // Locked "other" prevents modifications in "other" during the move operation
    {
    }

    auto get_data_comparator()
    {
        // Capturing the smart reference into the callback ensures a valid call destination.
        auto external_callback = [self = borrow_from_this()](int data_query) { return data_query == self->synced_m_.lock()->data; };
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
