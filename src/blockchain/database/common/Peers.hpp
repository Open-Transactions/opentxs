// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_plain_guarded.h>
#include <chrono>
#include <mutex>
#include <utility>

#include "internal/blockchain/database/common/Common.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/Types.hpp"
#include "opentxs/util/Allocator.hpp"
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

class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::database::common
{
class Peers
{
public:
    auto Confirm(
        const blockchain::Type chain,
        const network::blockchain::AddressID& id) noexcept -> void;
    auto Fail(
        const blockchain::Type chain,
        const network::blockchain::AddressID& id) noexcept -> void;
    auto Find(
        const blockchain::Type chain,
        const Protocol protocol,
        const Set<Transport>& onNetworks,
        const Set<Service>& withServices,
        const Set<network::blockchain::AddressID>& exclude) noexcept
        -> network::blockchain::Address;
    auto Good(
        const blockchain::Type chain,
        alloc::Default alloc,
        alloc::Default monotonic) noexcept
        -> Vector<network::blockchain::Address>;
    auto Import(Vector<network::blockchain::Address>&& peers) noexcept -> bool;
    auto Insert(network::blockchain::Address address) noexcept -> bool;
    auto Release(
        const blockchain::Type chain,
        const network::blockchain::AddressID& id) noexcept -> void;

    Peers(const api::Session& api, storage::lmdb::Database& lmdb) noexcept(
        false);

private:
    struct Data;

    using Addresses = Set<network::blockchain::AddressID>;
    using ChainIndexMap = Map<Chain, Addresses>;
    using ProtocolIndexMap = Map<Protocol, Addresses>;
    using ServiceIndexMap = Map<Service, Addresses>;
    using TypeIndexMap = Map<Transport, Addresses>;
    using ConnectedIndexMap = Map<network::blockchain::AddressID, Time>;
    using KnownGood = Addresses;
    using Untested = Addresses;
    using Retry = Map<sTime, Addresses>;
    using NextTimeout =
        Map<network::blockchain::AddressID, std::chrono::seconds>;
    using GuardedData = libguarded::plain_guarded<Data>;
    using Chains = Map<Chain, GuardedData>;

    struct Data {
        Addresses in_use_{};
        Addresses untested_{};
        Addresses known_good_{};
        Addresses failed_{};
        Retry retry_{};
        NextTimeout next_timeout_{};
    };

    const api::Session& api_;
    storage::lmdb::Database& lmdb_;
    const Log& log_;
    mutable std::mutex lock_;
    ChainIndexMap chains_;
    ProtocolIndexMap protocols_;
    ServiceIndexMap services_;
    TypeIndexMap networks_;
    ConnectedIndexMap connected_;
    mutable Chains data_;

    static auto exists(
        const Data& data,
        const network::blockchain::AddressID& id) noexcept -> bool;
    static auto next_timeout(Data&, network::blockchain::AddressID id) noexcept
        -> sTime;
    static auto retry_peers(Data& data) noexcept -> void;

    auto get_candidates(
        const blockchain::Type chain,
        const network::blockchain::Protocol protocol,
        const Set<network::blockchain::Transport>& onNetworks,
        const Set<network::blockchain::bitcoin::Service>& withServices,
        const Set<network::blockchain::AddressID>& exclude) const noexcept
        -> std::pair<Addresses, Addresses>;
    auto last_connected(const network::blockchain::AddressID& id) const noexcept
        -> Time;

    auto delete_peer(
        const blockchain::Type chain,
        const network::blockchain::AddressID& id) noexcept -> void;
    auto insert(
        const Lock& lock,
        const Vector<network::blockchain::Address>& peers) noexcept -> bool;
    auto load_address(const network::blockchain::AddressID& id) noexcept(false)
        -> network::blockchain::Address;
    template <typename Index, typename Map>
    auto read_index(
        const ReadView key,
        const ReadView value,
        Map& map) noexcept(false) -> bool;
};
}  // namespace opentxs::blockchain::database::common
