// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <chrono>
#include <future>

#include "internal/network/zeromq/socket/Pair.hpp"

#define TEST_ENDPOINT "inproc://opentxs/pairsocket_endpoint"

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
using namespace std::literals::chrono_literals;

class OPENTXS_EXPORT PairSocket : public ::testing::Test
{
public:
    const zmq::Context& context_;

    const ot::UnallocatedCString test_message_{"zeromq test message"};
    const ot::UnallocatedCString test_message2_{"zeromq test message 2"};

    ot::OTZMQPairSocket* pair_socket_;

    void pairSocketThread(
        const ot::UnallocatedCString& msg,
        std::promise<void>* promise);

    PairSocket();
    PairSocket(const PairSocket&) = delete;
    PairSocket(PairSocket&&) = delete;
    auto operator=(const PairSocket&) -> PairSocket& = delete;
    auto operator=(PairSocket&&) -> PairSocket& = delete;
};
}  // namespace ottest
