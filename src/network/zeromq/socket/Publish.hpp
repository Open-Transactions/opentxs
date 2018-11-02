// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::network::zeromq::socket::implementation
{
class PublishSocket final : virtual public zeromq::PublishSocket,
                            public Sender,
                            public zeromq::curve::implementation::Server
{
public:
    bool Publish(const std::string& data) const override
    {
        return Publish(Message::Factory(data));
    }
    bool Publish(const opentxs::Data& data) const override
    {
        return Publish(Message::Factory(data));
    }
    bool Publish(zeromq::Message& data) const override { return deliver(data); }

    ~PublishSocket();

private:
    friend opentxs::network::zeromq::PublishSocket;

    PublishSocket* clone() const override
    {
        return new PublishSocket(context_);
    }

    PublishSocket(const zeromq::Context& context);
    PublishSocket() = delete;
    PublishSocket(const PublishSocket&) = delete;
    PublishSocket(PublishSocket&&) = delete;
    PublishSocket& operator=(const PublishSocket&) = delete;
    PublishSocket& operator=(PublishSocket&&) = delete;
};
}  // namespace opentxs::network::zeromq::socket::implementation
