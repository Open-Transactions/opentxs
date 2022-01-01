// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstring>
#include <iosfwd>
#include <map>
#include <mutex>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/blockchain/database/common/Common.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Time.hpp"
#include "util/LMDB.hpp"

namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace storage
{
namespace lmdb
{
class LMDB;
}  // namespace lmdb
}  // namespace storage
}  // namespace opentxs

namespace opentxs::blockchain::database::common
{
class Peers
{
public:
    auto Find(
        const Chain chain,
        const Protocol protocol,
        const std::pmr::set<Type> onNetworks,
        const std::pmr::set<Service> withServices) const noexcept -> Address_p;

    auto Import(std::pmr::vector<Address_p> peers) noexcept -> bool;
    auto Insert(Address_p address) noexcept -> bool;

    Peers(const api::Session& api, storage::lmdb::LMDB& lmdb) noexcept(false);

private:
    using ChainIndexMap = std::pmr::map<Chain, std::pmr::set<std::string>>;
    using ProtocolIndexMap =
        std::pmr::map<Protocol, std::pmr::set<std::string>>;
    using ServiceIndexMap = std::pmr::map<Service, std::pmr::set<std::string>>;
    using TypeIndexMap = std::pmr::map<Type, std::pmr::set<std::string>>;
    using ConnectedIndexMap = std::pmr::map<std::string, Time>;

    const api::Session& api_;
    storage::lmdb::LMDB& lmdb_;
    mutable std::mutex lock_;
    ChainIndexMap chains_;
    ProtocolIndexMap protocols_;
    ServiceIndexMap services_;
    TypeIndexMap networks_;
    ConnectedIndexMap connected_;

    auto insert(const Lock& lock, std::pmr::vector<Address_p> peers) noexcept
        -> bool;
    auto load_address(const std::string& id) const noexcept(false) -> Address_p;
    template <typename Index, typename Map>
    auto read_index(
        const ReadView key,
        const ReadView value,
        Map& map) noexcept(false) -> bool
    {
        auto input = std::size_t{};

        if (sizeof(input) != key.size()) {
            throw std::runtime_error("Invalid key");
        }

        std::memcpy(&input, key.data(), key.size());
        map[static_cast<Index>(input)].emplace(value);

        return true;
    }
};
}  // namespace opentxs::blockchain::database::common
