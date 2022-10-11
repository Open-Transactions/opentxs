// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <future>
#include <utility>

#include "internal/network/zeromq/ListenCallback.hpp"

namespace ot = opentxs;
namespace zmq = opentxs::network::zeromq;

namespace ottest
{
class ListenCallback : public ::testing::Test
{
private:
    std::promise<ot::UnallocatedCString> promise_;

protected:
    const ot::UnallocatedCString test_message_;
    std::future<ot::UnallocatedCString> future_;
    ot::OTZMQListenCallback callback_;

    ListenCallback() noexcept
        : promise_()
        , test_message_("zeromq test message")
        , future_(promise_.get_future())
        , callback_(zmq::ListenCallback::Factory([&](auto&& input) -> void {
            promise_.set_value(ot::UnallocatedCString{input.at(0).Bytes()});
        }))
    {
    }
};

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
