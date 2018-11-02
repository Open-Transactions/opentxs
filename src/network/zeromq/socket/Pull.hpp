// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::network::zeromq::socket::implementation
{
class PullSocket final : virtual public zeromq::PullSocket,
                         public Receiver<zeromq::Message>,
                         zeromq::curve::implementation::Server
{
public:
    ~PullSocket();

private:
    friend opentxs::network::zeromq::PullSocket;

    const ListenCallback& callback_;

    PullSocket* clone() const override;
    bool have_callback() const override;

    void process_incoming(const Lock& lock, Message& message) override;

    PullSocket(
        const zeromq::Context& context,
        const Socket::Direction direction,
        const zeromq::ListenCallback& callback,
        const bool startThread);
    PullSocket(
        const zeromq::Context& context,
        const Socket::Direction direction,
        const zeromq::ListenCallback& callback);
    PullSocket(
        const zeromq::Context& context,
        const Socket::Direction direction);
    PullSocket() = delete;
    PullSocket(const PullSocket&) = delete;
    PullSocket(PullSocket&&) = delete;
    PullSocket& operator=(const PullSocket&) = delete;
    PullSocket& operator=(PullSocket&&) = delete;
};
}  // namespace opentxs::network::zeromq::socket::implementation
