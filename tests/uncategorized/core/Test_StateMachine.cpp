// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <atomic>
#include <chrono>

#include "core/StateMachine.hpp"

namespace ot = opentxs;

namespace ottest
{
using namespace std::literals::chrono_literals;

class Test_State_Machine : public ::testing::Test,
                           public ot::internal::StateMachine
{
public:
    std::atomic<int> step_;
    std::atomic<int> target_;
    std::atomic<int> counter_;

    auto callback() -> bool
    {
        while (step_.load() <= counter_.load()) { ot::Sleep(10us); }

        ++counter_;

        return counter_.load() < target_.load();
    }

    Test_State_Machine()
        : StateMachine([this] { return callback(); })
        , step_(0)
        , target_(0)
        , counter_(0)
    {
    }
    ~Test_State_Machine() override = default;
};

TEST_F(Test_State_Machine, stop_constructed)
{
    Stop().get();

    EXPECT_EQ(step_.load(), 0);
    EXPECT_EQ(target_.load(), 0);
    EXPECT_EQ(counter_.load(), 0);
}

TEST_F(Test_State_Machine, stop_running)
{
    counter_.store(0);
    step_.store(0);
    target_.store(1);

    EXPECT_TRUE(Trigger());
    EXPECT_EQ(counter_.load(), 0);

    auto future = Stop();

    ++step_;
    future.get();

    EXPECT_EQ(target_.load(), counter_.load());
}

TEST_F(Test_State_Machine, wait_constructed)
{
    Wait().get();

    EXPECT_EQ(step_.load(), 0);
    EXPECT_EQ(target_.load(), 0);
    EXPECT_EQ(counter_.load(), 0);
}

TEST_F(Test_State_Machine, wait_running)
{
    counter_.store(0);
    step_.store(0);
    target_.store(1);

    EXPECT_TRUE(Trigger());
    EXPECT_EQ(counter_.load(), 0);

    auto future = Wait();

    ++step_;
    future.get();

    EXPECT_EQ(target_.load(), counter_.load());
}

TEST_F(Test_State_Machine, stop_idle)
{
    counter_.store(0);
    step_.store(0);
    target_.store(1);

    EXPECT_TRUE(Trigger());
    EXPECT_EQ(counter_.load(), 0);

    auto future = Wait();

    ++step_;
    future.get();

    EXPECT_EQ(target_.load(), counter_.load());

    Stop().get();

    EXPECT_EQ(step_.load(), 1);
    EXPECT_EQ(target_.load(), 1);
    EXPECT_EQ(counter_.load(), 1);
}

TEST_F(Test_State_Machine, stop_stopped)
{
    Stop().get();
    Stop().get();

    EXPECT_EQ(step_.load(), 0);
    EXPECT_EQ(target_.load(), 0);
    EXPECT_EQ(counter_.load(), 0);
}

TEST_F(Test_State_Machine, wait_idle)
{
    counter_.store(0);
    step_.store(0);
    target_.store(1);

    EXPECT_TRUE(Trigger());
    EXPECT_EQ(counter_.load(), 0);

    auto future = Wait();

    ++step_;
    future.get();

    EXPECT_EQ(target_.load(), counter_.load());

    Wait().get();

    EXPECT_EQ(step_.load(), 1);
    EXPECT_EQ(target_.load(), 1);
    EXPECT_EQ(counter_.load(), 1);
}

TEST_F(Test_State_Machine, wait_stopped)
{
    Stop().get();
    Wait().get();

    EXPECT_EQ(step_.load(), 0);
    EXPECT_EQ(target_.load(), 0);
    EXPECT_EQ(counter_.load(), 0);
}

TEST_F(Test_State_Machine, trigger_idle)
{
    counter_.store(0);
    step_.store(0);
    target_.store(1);

    EXPECT_TRUE(Trigger());
    EXPECT_EQ(counter_.load(), 0);

    auto wait = Wait();
    ++step_;
    wait.get();

    EXPECT_EQ(target_.load(), counter_.load());

    counter_.store(0);
    step_.store(0);
    target_.store(3);

    EXPECT_TRUE(Trigger());
    EXPECT_EQ(counter_.load(), 0);

    ++step_;
    auto stop = Stop();
    ++step_;
    ++step_;
    stop.get();

    EXPECT_EQ(target_.load() - 2, counter_.load());
}

TEST_F(Test_State_Machine, trigger_running)
{
    step_.store(0);
    target_.store(1);

    EXPECT_TRUE(Trigger());
    EXPECT_EQ(counter_.load(), 0);

    auto future = Wait();

    EXPECT_TRUE(Trigger());

    ++step_;
    future.get();

    EXPECT_EQ(target_.load(), counter_.load());
}

TEST_F(Test_State_Machine, trigger_stopped)
{
    Stop().get();

    EXPECT_FALSE(Trigger());
}

TEST_F(Test_State_Machine, multiple_wait)
{
    counter_.store(0);
    step_.store(0);
    target_.store(5);

    EXPECT_TRUE(Trigger());
    EXPECT_EQ(counter_.load(), 0);

    auto wait1 = Wait();
    ++step_;
    auto wait2 = Wait();
    ++step_;
    auto wait3 = Wait();
    ++step_;
    ++step_;
    ++step_;

    wait1.get();
    wait2.get();
    wait3.get();

    EXPECT_EQ(target_.load(), counter_.load());
}

TEST_F(Test_State_Machine, multiple_stop)
{
    counter_.store(0);
    step_.store(0);
    target_.store(5);

    EXPECT_TRUE(Trigger());
    EXPECT_EQ(counter_.load(), 0);

    auto stop1 = Stop();
    auto stop2 = Stop();
    ++step_;
    auto stop3 = Stop();
    ++step_;
    ++step_;
    ++step_;
    ++step_;

    stop1.get();
    stop2.get();
    stop3.get();

    EXPECT_EQ(target_.load() - 4, counter_.load());
}
}  // namespace ottest
