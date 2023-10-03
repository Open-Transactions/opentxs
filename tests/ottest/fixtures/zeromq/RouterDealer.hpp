// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <atomic>
#include <chrono>

#include "internal/network/zeromq/socket/Dealer.hpp"
#include "internal/network/zeromq/socket/Router.hpp"

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
using namespace std::literals::chrono_literals;

class OPENTXS_EXPORT RouterDealer : public ::testing::Test
{
public:
    const zmq::Context& context_;
    ot::OTZMQRouterSocket* router_socket_;
    ot::OTZMQDealerSocket* dealer_socket_;
    const ot::UnallocatedCString test_message_;
    const ot::UnallocatedCString test_message2_;
    const ot::UnallocatedCString test_message3_;
    const ot::UnallocatedCString router_endpoint_;
    const ot::UnallocatedCString dealer_endpoint_;
    std::atomic_int callback_finished_count_;

    void requestSocketThread(
        const ot::UnallocatedCString& endpoint,
        const ot::UnallocatedCString& msg);

    void dealerSocketThread(
        const ot::UnallocatedCString& endpoint,
        const ot::UnallocatedCString& msg);

    RouterDealer();
    RouterDealer(const RouterDealer&) = delete;
    RouterDealer(RouterDealer&&) = delete;
    auto operator=(const RouterDealer&) -> RouterDealer& = delete;
    auto operator=(RouterDealer&&) -> RouterDealer& = delete;
};
}  // namespace ottest
