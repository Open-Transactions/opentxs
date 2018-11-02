// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::network::zeromq::socket::implementation
{
class PushSocket final : virtual public zeromq::PushSocket,
                         public Sender,
                         zeromq::curve::implementation::Client
{
public:
    bool Push(const std::string& data) const override
    {
        return Push(Message::Factory(data));
    }
    bool Push(const opentxs::Data& data) const override
    {
        return Push(Message::Factory(data));
    }
    bool Push(zeromq::Message& data) const override { return deliver(data); }

    ~PushSocket();

private:
    friend opentxs::network::zeromq::PushSocket;

    PushSocket* clone() const override
    {
        return new PushSocket(context_, direction_);
    }

    PushSocket(
        const zeromq::Context& context,
        const Socket::Direction direction);
    PushSocket() = delete;
    PushSocket(const PushSocket&) = delete;
    PushSocket(PushSocket&&) = delete;
    PushSocket& operator=(const PushSocket&) = delete;
    PushSocket& operator=(PushSocket&&) = delete;
};
}  // namespace opentxs::network::zeromq::socket::implementation
