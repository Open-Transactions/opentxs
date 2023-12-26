// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/zeromq/PushSubscribe.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <chrono>

#include "ottest/env/OTTestEnvironment.hpp"

namespace ottest
{
using namespace std::literals::chrono_literals;

PushSubscribe::PushSubscribe()
    : context_(OTTestEnvironment::GetOT().ZMQ())
    , test_message_("zeromq test message")
    , endpoint_1_("inproc://opentxs/test/push_subscribe_test")
    , endpoint_2_("inproc://opentxs/test/publish_subscribe_test")
    , counter_1_(0)
    , counter_2_(0)
    , counter_3_(0)
{
}
}  // namespace ottest
