// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/blockchain/database/common/Common.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/util/Bytes.hpp"
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
class Bulk;
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

namespace opentxs::blockchain::database::common
{
class BlockFilter
{
public:
    auto HaveFilter(const filter::Type type, const ReadView blockHash)
        const noexcept -> bool;
    auto HaveFilterHeader(const filter::Type type, const ReadView blockHash)
        const noexcept -> bool;
    auto LoadFilter(const filter::Type type, const ReadView blockHash)
        const noexcept -> std::unique_ptr<const opentxs::blockchain::GCS>;
    auto LoadFilterHash(
        const filter::Type type,
        const ReadView blockHash,
        const AllocateOutput filterHash) const noexcept -> bool;
    auto LoadFilterHeader(
        const filter::Type type,
        const ReadView blockHash,
        const AllocateOutput header) const noexcept -> bool;
    auto StoreFilterHeaders(
        const filter::Type type,
        const std::pmr::vector<FilterHeader>& headers) const noexcept -> bool;
    auto StoreFilters(
        const filter::Type type,
        std::pmr::vector<FilterData>& filters) const noexcept -> bool;
    auto StoreFilters(
        const filter::Type type,
        const std::pmr::vector<FilterHeader>& headers,
        const std::pmr::vector<FilterData>& filters) const noexcept -> bool;

    BlockFilter(
        const api::Session& api,
        storage::lmdb::LMDB& lmdb,
        Bulk& bulk) noexcept;

private:
    static const std::uint32_t blockchain_filter_header_version_{1};
    static const std::uint32_t blockchain_filter_headers_version_{1};
    static const std::uint32_t blockchain_filter_version_{1};
    static const std::uint32_t blockchain_filters_version_{1};

    const api::Session& api_;
    storage::lmdb::LMDB& lmdb_;
    Bulk& bulk_;

    static auto translate_filter(const filter::Type type) noexcept(false)
        -> Table;
    static auto translate_header(const filter::Type type) noexcept(false)
        -> Table;

    auto store(
        const Lock& lock,
        storage::lmdb::LMDB::Transaction& tx,
        const ReadView blockHash,
        const filter::Type type,
        const GCS& filter) const noexcept -> bool;
};
}  // namespace opentxs::blockchain::database::common
