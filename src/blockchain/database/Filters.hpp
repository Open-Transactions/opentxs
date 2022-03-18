// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/container/flat_set.hpp>
#include <algorithm>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <optional>
#include <utility>

#include "Proto.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "util/LMDB.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
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
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::database
{
class Filters
{
public:
    using Parent = node::internal::FilterDatabase;
    using Hash = Parent::Hash;
    using Header = Parent::Header;
    using Filter = Parent::Filter;

    auto CurrentHeaderTip(const cfilter::Type type) const noexcept
        -> block::Position;
    auto CurrentTip(const cfilter::Type type) const noexcept -> block::Position;
    auto HaveFilter(const cfilter::Type type, const block::Hash& block)
        const noexcept -> bool;
    auto HaveFilterHeader(const cfilter::Type type, const block::Hash& block)
        const noexcept -> bool;
    auto LoadFilter(const cfilter::Type type, const ReadView block)
        const noexcept -> std::unique_ptr<const blockchain::GCS>;
    auto LoadFilters(
        const cfilter::Type type,
        const Vector<block::pHash>& blocks) const noexcept
        -> Vector<std::unique_ptr<const GCS>>;
    auto LoadFilterHash(const cfilter::Type type, const ReadView block)
        const noexcept -> Hash;
    auto LoadFilterHeader(const cfilter::Type type, const ReadView block)
        const noexcept -> Hash;
    auto SetHeaderTip(const cfilter::Type type, const block::Position& position)
        const noexcept -> bool;
    auto SetTip(const cfilter::Type type, const block::Position& position)
        const noexcept -> bool;
    auto StoreFilters(
        const cfilter::Type type,
        const UnallocatedVector<Header>& headers,
        const UnallocatedVector<Filter>& filters,
        const block::Position& tip) const noexcept -> bool;
    auto StoreFilters(
        const cfilter::Type type,
        UnallocatedVector<Filter> filters) const noexcept -> bool;
    auto StoreHeaders(
        const cfilter::Type type,
        const ReadView previous,
        const UnallocatedVector<Header> headers) const noexcept -> bool;

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
