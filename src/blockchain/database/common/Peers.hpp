// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstring>
#include <iosfwd>
#include <mutex>
#include <stdexcept>

#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/blockchain/database/common/Common.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace p2p
{
class Address;
}  // namespace p2p
}  // namespace blockchain

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
    auto Find(
        const Chain chain,
        const Protocol protocol,
        const UnallocatedSet<Type> onNetworks,
        const UnallocatedSet<Service> withServices) const noexcept
        -> p2p::Address;

    auto Import(UnallocatedVector<p2p::Address>&& peers) noexcept -> bool;
    auto Insert(p2p::Address address) noexcept -> bool;

    Peers(const api::Session& api, storage::lmdb::Database& lmdb) noexcept(
        false);

private:
    using ChainIndexMap =
        UnallocatedMap<Chain, UnallocatedSet<UnallocatedCString>>;
    using ProtocolIndexMap =
        UnallocatedMap<Protocol, UnallocatedSet<UnallocatedCString>>;
    using ServiceIndexMap =
        UnallocatedMap<Service, UnallocatedSet<UnallocatedCString>>;
    using TypeIndexMap =
        UnallocatedMap<Type, UnallocatedSet<UnallocatedCString>>;
    using ConnectedIndexMap = UnallocatedMap<UnallocatedCString, Time>;

    const api::Session& api_;
    storage::lmdb::Database& lmdb_;
    mutable std::mutex lock_;
    ChainIndexMap chains_;
    ProtocolIndexMap protocols_;
    ServiceIndexMap services_;
    TypeIndexMap networks_;
    ConnectedIndexMap connected_;

    auto insert(const Lock& lock, const Vector<p2p::Address>& peers) noexcept
        -> bool;
    auto load_address(const UnallocatedCString& id) const noexcept(false)
        -> p2p::Address;
    template <typename Index, typename Map>
    auto read_index(
        const ReadView key,
        const ReadView value,
        Map& map) noexcept(false) -> bool
    {
        auto input = 0_uz;

        if (sizeof(input) != key.size()) {
            throw std::runtime_error("Invalid key");
        }

        std::memcpy(&input, key.data(), key.size());
        map[static_cast<Index>(input)].emplace(value);

        return true;
    }
};
}  // namespace opentxs::blockchain::database::common
