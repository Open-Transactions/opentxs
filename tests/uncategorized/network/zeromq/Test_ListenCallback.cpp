// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <future>
#include <utility>

#include "internal/network/zeromq/ListenCallback.hpp"
#include "ottest/fixtures/zeromq/ListenCallback.hpp"

namespace ot = opentxs;
namespace zmq = opentxs::network::zeromq;

namespace ottest
{
TEST_F(ListenCallback, ListenCallback_Process)
{
    auto message = [&] {
        auto out = zmq::Message{};
        out.AddFrame(test_message_);

        return out;
    }();
    callback_->Process(std::move(message));

    EXPECT_EQ(test_message_, future_.get());
}
}  // namespace ottest
