// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <BlockchainBlockHeader.pb.h>
#include <mutex>

#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/blockchain/database/Types.hpp"
#include "internal/blockchain/database/common/Common.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Types.hpp"

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
class Database;
class Transaction;
}  // namespace lmdb
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::database::common
{
class BlockHeader
{
public:
    auto Exists(const block::Hash& hash) const noexcept -> bool;
    auto Forget(const block::Hash& hash) const noexcept -> bool;
    auto Load(const block::Hash& hash) const noexcept(false)
        -> proto::BlockchainBlockHeader;
    auto Store(const UpdatedHeader& headers) const noexcept -> bool;

    BlockHeader(storage::lmdb::Database& lmdb, Bulk& bulk) noexcept(false);

private:
    storage::lmdb::Database& lmdb_;
    Bulk& bulk_;
    const int table_;
};
}  // namespace opentxs::blockchain::database::common
