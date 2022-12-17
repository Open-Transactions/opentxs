// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <span>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace block
{
class Hash;
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
}  // namespace lmdb
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::database::common
{
class Blocks
{
public:
    auto Exists(const block::Hash& block) const noexcept -> bool;
    auto Forget(const block::Hash& block) const noexcept -> bool;
    auto Load(
        blockchain::Type chain,
        const std::span<const block::Hash> hashes,
        alloc::Default alloc) const noexcept -> Vector<ReadView>;
    auto Store(const block::Hash& id, const ReadView bytes) const noexcept
        -> ReadView;

    Blocks(storage::lmdb::Database& lmdb, Bulk& bulk) noexcept;

    ~Blocks();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};
}  // namespace opentxs::blockchain::database::common
