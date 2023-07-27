// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <atomic>
#include <chrono>

#include "ottest/fixtures/core/StateMachine.hpp"

namespace ot = opentxs;

namespace ottest
{
TEST_F(StateMachine, stop_constructed)
{
    Stop().get();

    EXPECT_EQ(step_.load(), 0);
    EXPECT_EQ(target_.load(), 0);
    EXPECT_EQ(counter_.load(), 0);
}

TEST_F(StateMachine, stop_running)
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

TEST_F(StateMachine, wait_constructed)
{
    Wait().get();

    EXPECT_EQ(step_.load(), 0);
    EXPECT_EQ(target_.load(), 0);
    EXPECT_EQ(counter_.load(), 0);
}

TEST_F(StateMachine, wait_running)
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

TEST_F(StateMachine, stop_idle)
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

TEST_F(StateMachine, stop_stopped)
{
    Stop().get();
    Stop().get();

    EXPECT_EQ(step_.load(), 0);
    EXPECT_EQ(target_.load(), 0);
    EXPECT_EQ(counter_.load(), 0);
}

TEST_F(StateMachine, wait_idle)
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

TEST_F(StateMachine, wait_stopped)
{
    Stop().get();
    Wait().get();

    EXPECT_EQ(step_.load(), 0);
    EXPECT_EQ(target_.load(), 0);
    EXPECT_EQ(counter_.load(), 0);
}

TEST_F(StateMachine, trigger_idle)
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

TEST_F(StateMachine, trigger_running)
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

TEST_F(StateMachine, trigger_stopped)
{
    Stop().get();

    EXPECT_FALSE(Trigger());
}

TEST_F(StateMachine, multiple_wait)
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

TEST_F(StateMachine, multiple_stop)
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

