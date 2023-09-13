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

class OPENTXS_EXPORT PushSubscribe : public ::testing::Test
{
public:
    const zmq::Context& context_;
    const ot::UnallocatedCString test_message_;
    const ot::UnallocatedCString endpoint_1_;
    const ot::UnallocatedCString endpoint_2_;
    std::atomic<int> counter_1_;
    std::atomic<int> counter_2_;
    std::atomic<int> counter_3_;

    PushSubscribe();
};
}  // namespace ottest
