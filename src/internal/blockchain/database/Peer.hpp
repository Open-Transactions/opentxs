// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
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
    virtual auto Get(
        const p2p::Protocol protocol,
        const Set<p2p::Network>& onNetworks,
        const Set<p2p::Service>& withServices,
        const Set<identifier::Generic>& exclude) const noexcept
        -> p2p::Address = 0;

    virtual auto AddOrUpdate(p2p::Address address) noexcept -> bool = 0;
    virtual auto Import(Vector<p2p::Address> peers) noexcept -> bool = 0;

    virtual ~Peer() = default;
};
}  // namespace opentxs::blockchain::database
