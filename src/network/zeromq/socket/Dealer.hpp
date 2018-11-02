// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::network::zeromq::socket::implementation
{
class DealerSocket final : public _Bidirectional<zeromq::DealerSocket>,
                           zeromq::curve::implementation::Client
{
public:
    bool SetSocksProxy(const std::string& proxy) const override
    {
        return set_socks_proxy(proxy);
    }

    virtual ~DealerSocket();

protected:
    const ListenCallback& callback_;

    DealerSocket(
        const zeromq::Context& context,
        const Socket::Direction direction,
        const zeromq::ListenCallback& callback);

private:
    friend opentxs::network::zeromq::DealerSocket;

    DealerSocket* clone() const override;
    bool have_callback() const override { return true; }

    void process_incoming(const Lock& lock, Message& message) override;

    DealerSocket() = delete;
    DealerSocket(const DealerSocket&) = delete;
    DealerSocket(DealerSocket&&) = delete;
    DealerSocket& operator=(const DealerSocket&) = delete;
    DealerSocket& operator=(DealerSocket&&) = delete;
};
}  // namespace opentxs::network::zeromq::socket::implementation
