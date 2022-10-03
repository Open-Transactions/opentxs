// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace p2p
{
class Address;
}  // namespace p2p
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::database
{
class Peer
{
public:
    using Address = blockchain::p2p::Address;
    using Protocol = blockchain::p2p::Protocol;
    using Service = blockchain::p2p::Service;
    using Type = blockchain::p2p::Network;

    virtual auto Get(
        const Protocol protocol,
        const UnallocatedSet<Type> onNetworks,
        const UnallocatedSet<Service> withServices) const noexcept
        -> Address = 0;

    virtual auto AddOrUpdate(Address address) noexcept -> bool = 0;
    virtual auto Import(UnallocatedVector<Address> peers) noexcept -> bool = 0;

    virtual ~Peer() = default;
};
}  // namespace opentxs::blockchain::database
