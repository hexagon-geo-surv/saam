// SPDX-FileCopyrightText: Leica Geosystems AG
//
// SPDX-License-Identifier: MIT

#include <saam/synchronized.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <thread>

namespace saam::test
{

TEST(condition_test, wait_on_condition)

{
    saam::synchronized<int> synced_m(5);
    saam::synchronized<int>::condition above_5_condition(synced_m, [](const int &val) { return val > 5; });
    bool stop_thread = false;

    std::thread thread_worker([&]() {
        for (; !stop_thread;)
        {
            {
                auto locked_m = synced_m.commence_mut();
                (*locked_m)++;
            }
            above_5_condition.notify_all();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });

    {
        auto guard = synced_m.commence();
        above_5_condition.wait(guard);
        ASSERT_GT(*guard, 5);
    }

    stop_thread = true;
    thread_worker.join();
}

}  // namespace saam::test
