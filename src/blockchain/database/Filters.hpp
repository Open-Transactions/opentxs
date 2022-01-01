// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/container/flat_set.hpp>
#include <algorithm>
#include <cstdint>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "Proto.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "util/LMDB.hpp"

namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace database
{
namespace common
{
class Database;
}  // namespace common
}  // namespace database

class GCS;
}  // namespace blockchain

namespace storage
{
namespace lmdb
{
class LMDB;
}  // namespace lmdb
}  // namespace storage
}  // namespace opentxs

namespace opentxs::blockchain::database
{
class Filters
{
public:
    using Parent = node::internal::FilterDatabase;
    using Hash = Parent::Hash;
    using Header = Parent::Header;
    using Filter = Parent::Filter;

    auto CurrentHeaderTip(const filter::Type type) const noexcept
        -> block::Position;
    auto CurrentTip(const filter::Type type) const noexcept -> block::Position;
    auto HaveFilter(const filter::Type type, const block::Hash& block)
        const noexcept -> bool;
    auto HaveFilterHeader(const filter::Type type, const block::Hash& block)
        const noexcept -> bool;
    auto LoadFilter(const filter::Type type, const ReadView block)
        const noexcept -> std::unique_ptr<const blockchain::GCS>;
    auto LoadFilterHash(const filter::Type type, const ReadView block)
        const noexcept -> Hash;
    auto LoadFilterHeader(const filter::Type type, const ReadView block)
        const noexcept -> Hash;
    auto SetHeaderTip(const filter::Type type, const block::Position& position)
        const noexcept -> bool;
    auto SetTip(const filter::Type type, const block::Position& position)
        const noexcept -> bool;
    auto StoreFilters(
        const filter::Type type,
        const std::pmr::vector<Header>& headers,
        const std::pmr::vector<Filter>& filters,
        const block::Position& tip) const noexcept -> bool;
    auto StoreFilters(const filter::Type type, std::pmr::vector<Filter> filters)
        const noexcept -> bool;
    auto StoreHeaders(
        const filter::Type type,
        const ReadView previous,
        const std::pmr::vector<Header> headers) const noexcept -> bool;

    Filters(
        const api::Session& api,
        const common::Database& common,
        const storage::lmdb::LMDB& lmdb,
        const blockchain::Type chain) noexcept;

private:
    const api::Session& api_;
    const common::Database& common_;
    const storage::lmdb::LMDB& lmdb_;
    const block::Position blank_position_;
    mutable std::mutex lock_;

    auto import_genesis(const blockchain::Type type) const noexcept -> void;
};
}  // namespace opentxs::blockchain::database
