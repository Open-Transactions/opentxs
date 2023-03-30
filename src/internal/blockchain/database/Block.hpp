// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <span>

#include "internal/blockchain/database/Types.hpp"
#include "internal/util/storage/file/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace bitcoin
{
namespace block
{
class Block;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class Block;
class Hash;
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::database
{
class Block
{
public:
    virtual auto BlockDelete(const block::Hash& block) const noexcept
        -> bool = 0;
    virtual auto BlockExists(const block::Hash& block) const noexcept
        -> bool = 0;
    virtual auto BlockLoad(
        const std::span<const block::Hash> hashes,
        alloc::Default alloc,
        alloc::Default monotonic) const noexcept
        -> Vector<storage::file::Position> = 0;
    virtual auto BlockTip() const noexcept -> block::Position = 0;

    virtual auto BlockStore(
        const block::Hash& id,
        const ReadView bytes,
        alloc::Default monotonic) noexcept -> storage::file::Position = 0;
    virtual auto SetBlockTip(const block::Position& position) noexcept
        -> bool = 0;

    virtual ~Block() = default;
};
}  // namespace opentxs::blockchain::database
