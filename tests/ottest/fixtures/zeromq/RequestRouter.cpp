// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/zeromq/RequestRouter.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <chrono>
#include <span>
#include <string_view>
#include <utility>

#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/socket/Request.hpp"
#include "internal/util/Pimpl.hpp"
#include "ottest/env/OTTestEnvironment.hpp"

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
using namespace std::literals::chrono_literals;

RequestRouter::RequestRouter()
    : context_(OTTestEnvironment::GetOT().ZMQ())
{
}

void RequestRouter::requestSocketThread(const ot::UnallocatedCString& msg)
{
    auto requestSocket = context_.Internal().RequestSocket();

    ASSERT_NE(nullptr, &requestSocket.get());
    ASSERT_EQ(zmq::socket::Type::Request, requestSocket->Type());

    requestSocket->SetTimeouts(0ms, -1ms, 30000ms);
    requestSocket->Start(endpoint_);

    auto [result, message] = requestSocket->Send([&] {
        auto out = ot::network::zeromq::Message{};
        out.AddFrame(msg);

        return out;
    }());

    ASSERT_EQ(result, ot::otx::client::SendResult::VALID_REPLY);
    // RouterSocket removes the identity frame and RequestSocket removes the
    // delimiter.
    ASSERT_EQ(1, message.get().size());

    const auto messageString =
        ot::UnallocatedCString{message.Payload().begin()->Bytes()};
    ASSERT_EQ(msg, messageString);
}

void RequestRouter::requestSocketThreadMultipart()
{
    auto requestSocket = context_.Internal().RequestSocket();

    ASSERT_NE(nullptr, &requestSocket.get());
    ASSERT_EQ(zmq::socket::Type::Request, requestSocket->Type());

    requestSocket->SetTimeouts(0ms, -1ms, 30000ms);
    requestSocket->Start(endpoint_);

    auto multipartMessage = ot::network::zeromq::Message{};
    multipartMessage.AddFrame(test_message_);
    multipartMessage.StartBody();
    multipartMessage.AddFrame(test_message2_);
    multipartMessage.AddFrame(test_message3_);

    auto [result, message] = requestSocket->Send(std::move(multipartMessage));

    ASSERT_EQ(result, ot::otx::client::SendResult::VALID_REPLY);
    // RouterSocket removes the identity frame and RequestSocket removes the
    // delimiter.
    ASSERT_EQ(4, message.get().size());

    const auto messageHeader =
        ot::UnallocatedCString{message.Envelope().get()[0].Bytes()};

    ASSERT_EQ(test_message_, messageHeader);

    for (const auto& frame : message.Payload()) {
        bool match =
            frame.Bytes() == test_message2_ || frame.Bytes() == test_message3_;
        ASSERT_TRUE(match);
    }
}
}  // namespace ottest
