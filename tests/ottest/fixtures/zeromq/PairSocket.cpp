// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/zeromq/PairSocket.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <chrono>
#include <ctime>
#include <future>
#include <span>

#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/socket/Pair.hpp"
#include "internal/util/Pimpl.hpp"
#include "ottest/env/OTTestEnvironment.hpp"

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
using namespace std::literals::chrono_literals;

PairSocket::PairSocket()
    : context_(OTTestEnvironment::GetOT().ZMQ())
    , pair_socket_(nullptr)
{
}

void PairSocket::pairSocketThread(
    const ot::UnallocatedCString& message,
    std::promise<void>* promise)
{
    struct Cleanup {
        Cleanup(std::promise<void>& promise)
            : promise_(promise)
        {
        }

        ~Cleanup()
        {
            try {
                promise_.set_value();
            } catch (...) {
            }
        }

    private:
        std::promise<void>& promise_;
    };

    auto cleanup = Cleanup(*promise);
    bool callbackFinished = false;
    auto listenCallback = ot::network::zeromq::ListenCallback::Factory(
        [&callbackFinished,
         &message](ot::network::zeromq::Message&& msg) -> void {
            EXPECT_EQ(1, msg.get().size());
            const auto inputString =
                ot::UnallocatedCString{msg.Payload().begin()->Bytes()};

            EXPECT_EQ(message, inputString);

            callbackFinished = true;
        });

    ASSERT_NE(nullptr, &listenCallback.get());
    ASSERT_NE(nullptr, pair_socket_);

    auto pairSocket =
        context_.Internal().PairSocket(listenCallback, *pair_socket_);

    ASSERT_NE(nullptr, &pairSocket.get());
    ASSERT_EQ(zmq::socket::Type::Pair, pairSocket->Type());

    promise->set_value();
    auto end = std::time(nullptr) + 15;
    while (!callbackFinished && std::time(nullptr) < end) { ot::sleep(100ms); }

    ASSERT_TRUE(callbackFinished);
}
}  // namespace ottest
