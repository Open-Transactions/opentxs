// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/network/zeromq/curve/Client.hpp"
#include "opentxs/network/zeromq/socket/Sender.hpp"
#include "opentxs/util/Pimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class Dealer;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

using OTZMQDealerSocket = Pimpl<network::zeromq::socket::Dealer>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::socket
{
class OPENTXS_EXPORT Dealer : virtual public curve::Client,
                              virtual public Sender
{
public:
    virtual auto SetSocksProxy(const UnallocatedCString& proxy) const noexcept
        -> bool = 0;

    Dealer(const Dealer&) = delete;
    Dealer(Dealer&&) = delete;
    auto operator=(const Dealer&) -> Dealer& = delete;
    auto operator=(Dealer&&) -> Dealer& = delete;

    ~Dealer() override = default;

protected:
    Dealer() noexcept = default;

private:
    friend OTZMQDealerSocket;

    virtual auto clone() const noexcept -> Dealer* = 0;
};
}  // namespace opentxs::network::zeromq::socket
