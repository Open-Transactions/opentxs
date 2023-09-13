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

class OPENTXS_EXPORT DealerRouter : public ::testing::Test
{
public:
    const zmq::Context& context_;

    const ot::UnallocatedCString test_message_{"zeromq test message"};
    const ot::UnallocatedCString test_message2_{"zeromq test message 2"};
    const ot::UnallocatedCString test_message3_{"zeromq test message 3"};

    const ot::UnallocatedCString endpoint_{
        "inproc://opentxs/test/dealer_router_test"};
    const ot::UnallocatedCString endpoint2_{
        "inproc://opentxs/test/dealer_router_test2"};

    std::atomic_int callback_finished_count_{0};

    int callback_count_{0};

    void dealerSocketThread(const ot::UnallocatedCString& msg);
    void routerSocketThread(const ot::UnallocatedCString& endpoint);

    DealerRouter();
};
}  // namespace ottest
