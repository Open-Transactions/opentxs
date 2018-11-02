// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/network/zeromq/SubscribeSocket.hpp"

#include "network/zeromq/curve/Client.hpp"
#include "Receiver.hpp"
#include "Socket.hpp"

namespace opentxs::network::zeromq::socket::implementation
{
class SubscribeSocket : virtual public zeromq::SubscribeSocket,
                        public Receiver<zeromq::Message>,
                        zeromq::curve::implementation::Client
{
public:
    bool SetSocksProxy(const std::string& proxy) const override;

    virtual ~SubscribeSocket();

protected:
    const ListenCallback& callback_;

    SubscribeSocket(
        const zeromq::Context& context,
        const zeromq::ListenCallback& callback);

private:
    friend opentxs::network::zeromq::SubscribeSocket;

    SubscribeSocket* clone() const override;
    bool have_callback() const override;

    void init() override;
    void process_incoming(const Lock& lock, Message& message) override;

    SubscribeSocket() = delete;
    SubscribeSocket(const SubscribeSocket&) = delete;
    SubscribeSocket(SubscribeSocket&&) = delete;
    SubscribeSocket& operator=(const SubscribeSocket&) = delete;
    SubscribeSocket& operator=(SubscribeSocket&&) = delete;
};
}  // namespace opentxs::network::zeromq::socket::implementation
