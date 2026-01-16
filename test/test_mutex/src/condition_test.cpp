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

TEST(condition_test, wait_on_condition)
{
    saam::synchronized<int> synced_m(5);
    saam::condition value_changed(synced_m);
    bool stop_thread = false;

    std::thread thread_worker([&]() {
        for (; !stop_thread;)
        {
            {
                auto locked_m = synced_m.lock_mut();
                (*locked_m)++;
            }
            value_changed.notify();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });

    {
        auto sentinel = synced_m.lock_mut();
        value_changed.wait(sentinel, [](const int &val) { return val > 5; }, {std::chrono::milliseconds(100)});
        ASSERT_GT(*sentinel, 5);
    }

    stop_thread = true;
    thread_worker.join();
}

}  // namespace saam::test
