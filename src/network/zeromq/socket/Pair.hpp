// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/network/zeromq/PairSocket.hpp"

#include "Bidirectional.hpp"
#include "Socket.hpp"

namespace opentxs::network::zeromq::socket::implementation
{
class PairSocket final : virtual public zeromq::PairSocket, public Bidirectional
{
public:
    const std::string& Endpoint() const override;
    bool Send(const std::string& data) const override;
    bool Send(const opentxs::Data& data) const override;
    bool Send(network::zeromq::Message& data) const override;
    bool Start(const std::string& endpoint) const override { return false; }

    ~PairSocket();

private:
    friend opentxs::network::zeromq::PairSocket;
    friend zeromq::implementation::Proxy;

    const ListenCallback& callback_;
    const std::string endpoint_;

    PairSocket* clone() const override;
    bool have_callback() const override;
    void process_incoming(const Lock& lock, Message& message) override;

    void init() override;

    PairSocket(
        const zeromq::Context& context,
        const zeromq::ListenCallback& callback,
        const std::string& endpoint,
        const Socket::Direction direction,
        const bool startThread);
    PairSocket(
        const zeromq::Context& context,
        const zeromq::ListenCallback& callback,
        const bool startThread = true);
    PairSocket(
        const zeromq::ListenCallback& callback,
        const zeromq::PairSocket& peer,
        const bool startThread = true);
    PairSocket(
        const zeromq::Context& context,
        const zeromq::ListenCallback& callback,
        const std::string& endpoint);
    PairSocket() = delete;
    PairSocket(const PairSocket&) = delete;
    PairSocket(PairSocket&&) = delete;
    PairSocket& operator=(const PairSocket&) = delete;
    PairSocket& operator=(PairSocket&&) = delete;
};
}  // namespace opentxs::network::zeromq::socket::implementation
