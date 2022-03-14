// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <mutex>

#include "Proto.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/blockchain/database/common/Common.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "serialization/protobuf/BlockchainBlockHeader.pb.h"
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
namespace block
{
class Header;
}  // namespace block

namespace database
{
namespace common
{
class Bulk;
}  // namespace common
}  // namespace database
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

namespace opentxs::blockchain::database::common
{
class BlockHeader
{
public:
    auto Exists(const opentxs::blockchain::block::Hash& hash) const noexcept
        -> bool;
    auto Load(const opentxs::blockchain::block::Hash& hash) const
        noexcept(false) -> proto::BlockchainBlockHeader;
    auto Store(const opentxs::blockchain::block::Header& header) const noexcept
        -> bool;
    auto Store(const UpdatedHeader& headers) const noexcept -> bool;

    BlockHeader(storage::lmdb::LMDB& lmdb, Bulk& bulk) noexcept(false);

private:
    storage::lmdb::LMDB& lmdb_;
    Bulk& bulk_;
    const int table_;

    auto store(
        const Lock& lock,
        bool clearLocal,
        storage::lmdb::LMDB::Transaction& tx,
        const opentxs::blockchain::block::Header& header) const noexcept
        -> bool;
};
}  // namespace opentxs::blockchain::database::common
