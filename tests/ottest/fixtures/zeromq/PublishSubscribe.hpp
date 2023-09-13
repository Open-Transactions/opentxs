// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <atomic>
#include <chrono>

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
using namespace std::literals::chrono_literals;

class OPENTXS_EXPORT PublishSubscribe : public ::testing::Test
{
public:
    const zmq::Context& context_;

    const ot::UnallocatedCString test_message_{"zeromq test message"};
    const ot::UnallocatedCString test_message2_{"zeromq test message 2"};

    const ot::UnallocatedCString endpoint_{
        "inproc://opentxs/test/publish_subscribe_test"};
    const ot::UnallocatedCString endpoint2_{
        "inproc://opentxs/test/publish_subscribe_test2"};

    std::atomic_int callback_finished_count_{0};
    std::atomic_int subscribe_thread_started_count_{0};
    std::atomic_int publish_thread_started_count_{0};

    int subscribe_thread_count_{0};
    int callback_count_{0};

    void subscribeSocketThread(
        const ot::UnallocatedSet<ot::UnallocatedCString>& endpoints,
        const ot::UnallocatedSet<ot::UnallocatedCString>& msgs);
    void publishSocketThread(
        const ot::UnallocatedCString& endpoint,
        const ot::UnallocatedCString& msg);

    PublishSubscribe();
};
}  // namespace ottest
