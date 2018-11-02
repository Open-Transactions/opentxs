// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::network::zeromq::socket::implementation
{
class RequestSocket final : virtual public zeromq::RequestSocket,
                            public Sender,
                            zeromq::curve::implementation::Client
{
public:
    SendResult SendRequest(opentxs::Data& message) const override;
    SendResult SendRequest(const std::string& message) const override;
    SendResult SendRequest(zeromq::Message& message) const override;
    bool SetSocksProxy(const std::string& proxy) const override;

    ~RequestSocket();

private:
    friend opentxs::network::zeromq::RequestSocket;

    RequestSocket* clone() const override;
    bool wait(const Lock& lock) const;

    RequestSocket(const zeromq::Context& context);
    RequestSocket() = delete;
    RequestSocket(const RequestSocket&) = delete;
    RequestSocket(RequestSocket&&) = delete;
    RequestSocket& operator=(const RequestSocket&) = delete;
    RequestSocket& operator=(RequestSocket&&) = delete;
};
}  // namespace opentxs::network::zeromq::socket::implementation
