// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_plain_guarded.h>
#include <cs_shared_guarded.h>
#include <chrono>
#include <functional>
#include <future>
#include <memory>
#include <shared_mutex>
#include <span>
#include <utility>

#include "internal/blockchain/database/common/Common.hpp"
#include "internal/util/storage/lmdb/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
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
namespace internal
{
class Session;
}  // namespace internal

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
    auto IsReady() const noexcept -> bool;
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

    ~Peers();

private:
    struct Data;
    struct Index;

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
    using GuardedIndex = libguarded::plain_guarded<Index>;
    using Chains = Map<Chain, GuardedIndex>;
    using GuardedData = libguarded::shared_guarded<Data, std::shared_mutex>;

    struct Index {
        Addresses in_use_{};
        Addresses untested_{};
        Addresses known_good_{};
        Addresses failed_{};
        Retry retry_{};
        NextTimeout next_timeout_{};
    };

    struct Data {
        ChainIndexMap chains_{};
        ProtocolIndexMap protocols_{};
        ServiceIndexMap services_{};
        TypeIndexMap networks_{};
        ConnectedIndexMap connected_{};
        Chains chain_index_{};
    };

    const Log& log_;
    const api::Session& api_;
    storage::lmdb::Database& lmdb_;
    GuardedData data_;
    std::shared_future<GuardedData&> future_;

    static auto exists(
        const Index& data,
        const network::blockchain::AddressID& id) noexcept -> bool;
    static auto next_timeout(Index&, network::blockchain::AddressID id) noexcept
        -> sTime;
    static auto retry_peers(Index& data) noexcept -> void;

    auto get() const noexcept -> const GuardedData& { return future_.get(); }
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
    auto get() noexcept -> GuardedData& { return future_.get(); }
    auto init(
        std::shared_ptr<const api::internal::Session> api,
        std::shared_ptr<std::promise<GuardedData&>> promise) noexcept -> void;
    auto init_chains(
        const api::Session& api,
        const Time now,
        Data& data,
        std::span<const std::pair<const Addresses*, GuardedIndex*>>
            chains) noexcept -> void;
    auto init_tables(
        const api::Session& api,
        std::span<const std::pair<Table, storage::lmdb::ReadCallback>>
            work) noexcept -> void;
    auto insert(
        Data& data,
        const Vector<network::blockchain::Address>& peers) noexcept -> bool;
    auto load_address(const network::blockchain::AddressID& id) noexcept(false)
        -> network::blockchain::Address;
    template <typename Key, typename Map>
    auto read_index(
        const ReadView key,
        const ReadView value,
        Map& map) noexcept(false) -> bool;
};
}  // namespace opentxs::blockchain::database::common
