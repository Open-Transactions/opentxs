// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::network::zeromq::socket::implementation
{
class RouterSocket final : public _Bidirectional<zeromq::RouterSocket>,
                           zeromq::curve::implementation::Client,
                           zeromq::curve::implementation::Server
{
public:
    bool SetSocksProxy(const std::string& proxy) const override
    {
        return set_socks_proxy(proxy);
    }

    virtual ~RouterSocket();

protected:
    const ListenCallback& callback_;

    RouterSocket(
        const zeromq::Context& context,
        const Socket::Direction direction,
        const zeromq::ListenCallback& callback);

private:
    friend opentxs::network::zeromq::RouterSocket;

    RouterSocket* clone() const override;
    bool have_callback() const override { return true; }

    void process_incoming(const Lock& lock, Message& message) override;

    RouterSocket() = delete;
    RouterSocket(const RouterSocket&) = delete;
    RouterSocket(RouterSocket&&) = delete;
    RouterSocket& operator=(const RouterSocket&) = delete;
    RouterSocket& operator=(RouterSocket&&) = delete;
};
}  // namespace opentxs::network::zeromq::socket::implementation
