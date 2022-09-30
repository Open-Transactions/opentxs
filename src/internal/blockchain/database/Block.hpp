// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/blockchain/database/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"

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
    virtual auto BlockExists(const block::Hash& block) const noexcept
        -> bool = 0;
    virtual auto BlockLoadBitcoin(const block::Hash& block) const noexcept
        -> std::shared_ptr<const bitcoin::block::Block> = 0;
    virtual auto BlockTip() const noexcept -> block::Position = 0;

    virtual auto BlockStore(const block::Block& block) noexcept -> bool = 0;
    virtual auto SetBlockTip(const block::Position& position) noexcept
        -> bool = 0;

    virtual ~Block() = default;
};
}  // namespace opentxs::blockchain::database
