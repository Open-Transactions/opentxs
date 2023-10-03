// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/zeromq/RequestReply.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <chrono>
#include <ctime>
#include <span>

#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/ReplyCallback.hpp"
#include "internal/network/zeromq/socket/Reply.hpp"
#include "internal/network/zeromq/socket/Request.hpp"
#include "internal/util/Pimpl.hpp"
#include "ottest/env/OTTestEnvironment.hpp"

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
using namespace std::literals::chrono_literals;

RequestReply::RequestReply()
    : context_(OTTestEnvironment::GetOT().ZMQ())
{
}

void RequestReply::requestSocketThread(const ot::UnallocatedCString& msg)
{
    auto requestSocket = context_.Internal().RequestSocket();

    ASSERT_NE(nullptr, &requestSocket.get());
    ASSERT_EQ(zmq::socket::Type::Request, requestSocket->Type());

    requestSocket->SetTimeouts(0ms, -1ms, 30000ms);
    requestSocket->Start(endpoint_);

    auto [result, message] = requestSocket->Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(msg);

        return out;
    }());

    ASSERT_EQ(result, ot::otx::client::SendResult::VALID_REPLY);

    const auto messageString =
        ot::UnallocatedCString{message.Payload().begin()->Bytes()};
    ASSERT_EQ(msg, messageString);
}

void RequestReply::replySocketThread(const ot::UnallocatedCString& endpoint)
{
    bool replyReturned{false};

    auto replyCallback = zmq::ReplyCallback::Factory(
        [this,
         &replyReturned](zmq::Message&& input) -> ot::network::zeromq::Message {
            const auto inputString =
                ot::UnallocatedCString{input.Payload().begin()->Bytes()};
            bool match =
                inputString == test_message2_ || inputString == test_message3_;
            EXPECT_TRUE(match);

            auto reply = ot::network::zeromq::reply_to_message(input);
            reply.AddFrame(inputString);
            replyReturned = true;
            return reply;
        });

    ASSERT_NE(nullptr, &replyCallback.get());

    auto replySocket = context_.Internal().ReplySocket(
        replyCallback, zmq::socket::Direction::Bind);

    ASSERT_NE(nullptr, &replySocket.get());
    ASSERT_EQ(zmq::socket::Type::Reply, replySocket->Type());

    replySocket->SetTimeouts(0ms, 30000ms, -1ms);
    replySocket->Start(endpoint);

    auto end = std::time(nullptr) + 15;
    while (!replyReturned && std::time(nullptr) < end) { ot::Sleep(100ms); }

    EXPECT_TRUE(replyReturned);
}
}  // namespace ottest
