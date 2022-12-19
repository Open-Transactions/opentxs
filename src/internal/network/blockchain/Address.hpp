// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/network/blockchain/Address.hpp"

#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace proto
{
class BlockchainPeerAddress;
}  // namespace proto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::blockchain::internal
{
class Address
{
public:
    virtual auto Incoming() const noexcept -> bool = 0;
    virtual auto PreviousLastConnected() const noexcept -> Time = 0;
    virtual auto PreviousServices() const noexcept -> Set<bitcoin::Service> = 0;
    virtual auto Serialize(proto::BlockchainPeerAddress& out) const noexcept
        -> bool = 0;

    virtual auto AddService(const bitcoin::Service service) noexcept
        -> void = 0;
    virtual auto RemoveService(const bitcoin::Service service) noexcept
        -> void = 0;
    virtual auto SetIncoming(bool value) noexcept -> void = 0;
    virtual auto SetLastConnected(const Time& time) noexcept -> void = 0;
    virtual auto SetServices(const Set<bitcoin::Service>& services) noexcept
        -> void = 0;

    virtual ~Address() = default;
};
}  // namespace opentxs::network::blockchain::internal
