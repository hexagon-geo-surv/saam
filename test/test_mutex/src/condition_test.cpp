// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#include <saam/condition.hpp>
#include <saam/synchronized.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <thread>

namespace saam::test
{

class condition_test : public ::testing::Test
{
  public:
    void SetUp() override
    {
    }
};

TEST_F(condition_test, cond)
{
    saam::synchronized<double> synced_m(42.5);
    saam::condition value_changed(synced_m);

    std::thread thread_worker([&]() {
        for (;;)
        {
            double new_value;
            {
                auto locked_m = synced_m.lock_mut();
                new_value = ++(*locked_m);
            }
            value_changed.notify();
            if (new_value > 10)
            {
                break;
            }
        }
    });

    {
        auto sentinel = synced_m.lock();
        value_changed.wait(sentinel, [](const double &val) { return val > 5; }, {std::chrono::milliseconds(100)});
        ASSERT_GE(*sentinel, 5);
    }

    // auto sentinel2 = synced_m.lock_mut();
    // value_changed.wait(sentinel2, [](const int &val) { return val > 5; });
    // *sentinel2 += 10;
    // ASSERT_GE(*sentinel2, 15);

    thread_worker.join();
}

}  // namespace saam::test
