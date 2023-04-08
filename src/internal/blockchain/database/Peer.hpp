// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "internal/blockchain/database/common/Common.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace blockchain
{
class Address;
}  // namespace blockchain
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::database
{
class Peer
{
public:
    virtual auto Good(alloc::Default alloc, alloc::Default monotonic)
        const noexcept -> Vector<network::blockchain::Address> = 0;

    virtual auto AddOrUpdate(network::blockchain::Address address) noexcept
        -> bool = 0;
    virtual auto Confirm(const network::blockchain::AddressID& id) noexcept
        -> void = 0;
    virtual auto Fail(const network::blockchain::AddressID& id) noexcept
        -> void = 0;
    virtual auto Get(
        const Protocol protocol,
        const Set<Transport>& onNetworks,
        const Set<Service>& withServices,
        const Set<network::blockchain::AddressID>& exclude) noexcept
        -> network::blockchain::Address = 0;
    virtual auto Import(Vector<network::blockchain::Address> peers) noexcept
        -> bool = 0;
    virtual auto PeerIsReady() const noexcept -> bool = 0;
    virtual auto Release(const network::blockchain::AddressID& id) noexcept
        -> void = 0;

    virtual ~Peer() = default;
};
}  // namespace opentxs::blockchain::database
