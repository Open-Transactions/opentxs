// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <shared_mutex>

#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Container.hpp"
#include "util/LMDB.hpp"
#include "util/MappedFileStorage.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace blockchain
{
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
class Blocks
{
public:
    using Hash = opentxs::blockchain::block::Hash;
    using pHash = opentxs::blockchain::block::pHash;

    auto Exists(const Hash& block) const noexcept -> bool;
    auto Load(const Hash& block) const noexcept -> BlockReader;
    auto Store(const Hash& block, const std::size_t bytes) const noexcept
        -> BlockWriter;

    Blocks(storage::lmdb::LMDB& lmdb, Bulk& bulk) noexcept;

    ~Blocks();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};
}  // namespace opentxs::blockchain::database::common
