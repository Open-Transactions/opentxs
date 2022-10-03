// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "internal/blockchain/database/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/util/Allocator.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace bitcoin
{
namespace block
{
class Header;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class Hash;
class Header;
}  // namespace block

namespace node
{
class UpdateTransaction;
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::database
{
class Header
{
public:
    // Throws std::out_of_range if no block at that position
    virtual auto BestBlock(const block::Height position) const noexcept(false)
        -> block::Hash = 0;
    virtual auto CurrentBest() const noexcept
        -> std::unique_ptr<block::Header> = 0;
    virtual auto CurrentCheckpoint() const noexcept -> block::Position = 0;
    virtual auto DisconnectedHashes() const noexcept -> DisconnectedList = 0;
    virtual auto HasDisconnectedChildren(const block::Hash& hash) const noexcept
        -> bool = 0;
    virtual auto HaveCheckpoint() const noexcept -> bool = 0;
    virtual auto HeaderExists(const block::Hash& hash) const noexcept
        -> bool = 0;
    virtual auto IsSibling(const block::Hash& hash) const noexcept -> bool = 0;
    // Throws std::out_of_range if the header does not exist
    virtual auto LoadHeader(const block::Hash& hash) const noexcept(false)
        -> std::unique_ptr<block::Header> = 0;
    virtual auto RecentHashes(alloc::Default alloc = {}) const noexcept
        -> HashVector = 0;
    virtual auto SiblingHashes() const noexcept -> Hashes = 0;
    // Returns null pointer if the header does not exist
    virtual auto TryLoadBitcoinHeader(const block::Hash& hash) const noexcept
        -> std::unique_ptr<bitcoin::block::Header> = 0;
    // Returns null pointer if the header does not exist
    virtual auto TryLoadHeader(const block::Hash& hash) const noexcept
        -> std::unique_ptr<block::Header> = 0;

    virtual auto ApplyUpdate(const node::UpdateTransaction& update) noexcept
        -> bool = 0;
    virtual auto ReportHeaderTip() noexcept -> void = 0;

    virtual ~Header() = default;
};
}  // namespace opentxs::blockchain::database
