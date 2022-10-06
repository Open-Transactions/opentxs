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

#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/blockchain/database/Cfilter.hpp"
#include "internal/blockchain/database/Types.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Input.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Hash.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Header.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
class Position;
}  // namespace block

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
class Database;
}  // namespace lmdb
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::database
{
class Filters
{
public:
    using Parent = database::Cfilter;
    using CFHeaderParams = Parent::CFHeaderParams;
    using CFilterParams = Parent::CFilterParams;

    auto CurrentHeaderTip(const cfilter::Type type) const noexcept
        -> block::Position;
    auto CurrentTip(const cfilter::Type type) const noexcept -> block::Position;
    auto HaveFilter(const cfilter::Type type, const block::Hash& block)
        const noexcept -> bool;
    auto HaveFilterHeader(const cfilter::Type type, const block::Hash& block)
        const noexcept -> bool;
    auto LoadFilter(
        const cfilter::Type type,
        const ReadView block,
        alloc::Default alloc) const noexcept -> blockchain::GCS;
    auto LoadFilters(
        const cfilter::Type type,
        const Vector<block::Hash>& blocks) const noexcept -> Vector<GCS>;
    auto LoadFilterHash(const cfilter::Type type, const ReadView block)
        const noexcept -> cfilter::Hash;
    auto LoadFilterHeader(const cfilter::Type type, const ReadView block)
        const noexcept -> cfilter::Header;
    auto SetHeaderTip(const cfilter::Type type, const block::Position& position)
        const noexcept -> bool;
    auto SetTip(const cfilter::Type type, const block::Position& position)
        const noexcept -> bool;
    auto StoreFilters(
        const cfilter::Type type,
        const Vector<CFHeaderParams>& headers,
        const Vector<CFilterParams>& filters,
        const block::Position& tip) const noexcept -> bool;
    auto StoreFilters(const cfilter::Type type, Vector<CFilterParams> filters)
        const noexcept -> bool;
    auto StoreHeaders(
        const cfilter::Type type,
        const ReadView previous,
        const Vector<CFHeaderParams> headers) const noexcept -> bool;

    Filters(
        const api::Session& api,
        const common::Database& common,
        const storage::lmdb::Database& lmdb,
        const blockchain::Type chain) noexcept;

private:
    const api::Session& api_;
    const common::Database& common_;
    const storage::lmdb::Database& lmdb_;
    const block::Position blank_position_;
    mutable std::mutex lock_;

    auto import_genesis(const blockchain::Type type) const noexcept -> void;
};
}  // namespace opentxs::blockchain::database
