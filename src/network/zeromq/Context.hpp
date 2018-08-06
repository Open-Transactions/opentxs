// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/network/zeromq/Context.hpp"

namespace opentxs::network::zeromq::implementation
{
class Context : virtual public zeromq::Context
{
public:
    operator void*() const override;

    OTZMQDealerSocket DealerSocket(
        const ListenCallback& callback,
        const bool client) const override;
    OTZMQSubscribeSocket PairEventListener(
        const PairEventCallback& callback) const override;
    OTZMQPairSocket PairSocket(const opentxs::network::zeromq::ListenCallback&
                                   callback) const override;
    OTZMQPairSocket PairSocket(
        const opentxs::network::zeromq::ListenCallback& callback,
        const opentxs::network::zeromq::PairSocket& peer) const override;
    OTZMQPairSocket PairSocket(
        const opentxs::network::zeromq::ListenCallback& callback,
        const std::string& endpoint) const override;
    OTZMQProxy Proxy(
        network::zeromq::Socket& frontend,
        network::zeromq::Socket& backend) const override;
    OTZMQPublishSocket PublishSocket() const override;
    OTZMQPullSocket PullSocket(const bool client) const override;
    OTZMQPullSocket PullSocket(
        const ListenCallback& callback,
        const bool client) const override;
    OTZMQPushSocket PushSocket(const bool client) const override;
    OTZMQReplySocket ReplySocket(
        const ReplyCallback& callback,
        const bool client) const override;
    OTZMQRequestSocket RequestSocket() const override;
    OTZMQRouterSocket RouterSocket(
        const ListenCallback& callback,
        const bool client) const override;
    OTZMQSubscribeSocket SubscribeSocket(
        const ListenCallback& callback) const override;

    ~Context();

private:
    friend network::zeromq::Context;

    void* context_{nullptr};

    Context* clone() const override;

    Context();
    Context(const Context&) = delete;
    Context(Context&&) = delete;
    Context& operator=(const Context&) = delete;
    Context& operator=(Context&&) = delete;
};
}  // namespace opentxs::network::zeromq::implementation
