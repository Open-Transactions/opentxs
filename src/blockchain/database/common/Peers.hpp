// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <mutex>

#include "internal/blockchain/database/common/Common.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace network
{
namespace blockchain
{
class Address;
}  // namespace blockchain
}  // namespace network

namespace storage
{
namespace lmdb
{
class Database;
}  // namespace lmdb
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::database::common
{
class Peers
{
public:
    using AddressID = identifier::Generic;
    using Addresses = Set<AddressID>;

    auto Find(
        const blockchain::Type chain,
        const Protocol protocol,
        const Set<Transport>& onNetworks,
        const Set<Service>& withServices,
        const Set<identifier::Generic>& exclude) const noexcept
        -> network::blockchain::Address;

    auto Import(Vector<network::blockchain::Address>&& peers) noexcept -> bool;
    auto Insert(network::blockchain::Address address) noexcept -> bool;

    Peers(const api::Session& api, storage::lmdb::Database& lmdb) noexcept(
        false);

private:
    using ChainIndexMap = Map<Chain, Set<AddressID>>;
    using ProtocolIndexMap = Map<Protocol, Set<AddressID>>;
    using ServiceIndexMap = Map<Service, Set<AddressID>>;
    using TypeIndexMap = Map<Transport, Set<AddressID>>;
    using ConnectedIndexMap = Map<AddressID, Time>;

    const api::Session& api_;
    storage::lmdb::Database& lmdb_;
    mutable std::mutex lock_;
    ChainIndexMap chains_;
    ProtocolIndexMap protocols_;
    ServiceIndexMap services_;
    TypeIndexMap networks_;
    ConnectedIndexMap connected_;

    auto insert(
        const Lock& lock,
        const Vector<network::blockchain::Address>& peers) noexcept -> bool;
    auto load_address(const AddressID& id) const noexcept(false)
        -> network::blockchain::Address;
    template <typename Index, typename Map>
    auto read_index(
        const ReadView key,
        const ReadView value,
        Map& map) noexcept(false) -> bool;
};
}  // namespace opentxs::blockchain::database::common
