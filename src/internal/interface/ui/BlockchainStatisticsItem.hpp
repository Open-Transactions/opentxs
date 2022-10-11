// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/interface/ui/ListRow.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
class BlockchainStatisticsItem;
}  // namespace ui

using OTUIBlockchainStatisticsItem = SharedPimpl<ui::BlockchainStatisticsItem>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
/**
  This model represents the statistics for a single blockchain in the wallet.
  There is a model like this for each row in the BlockchainStatistics model.
*/
class BlockchainStatisticsItem : virtual public ListRow
{
public:
    using Position = blockchain::block::Height;

    /// Returns the number of active peers for this blockchain.
    virtual auto ActivePeers() const noexcept -> std::size_t = 0;
    /// Returns the current balance for this blockchain.
    virtual auto Balance() const noexcept -> UnallocatedCString = 0;
    /// Returns the number of blocks currently in the block download queue.
    virtual auto BlockDownloadQueue() const noexcept -> std::size_t = 0;
    /// Returns the blockchain type (Bitcoin, Ethereum, etc).
    virtual auto Chain() const noexcept -> blockchain::Type = 0;
    /// Returns the number of connected peers for this blockchain.
    virtual auto ConnectedPeers() const noexcept -> std::size_t = 0;
    /// Returns the number of filters downloaded so far for this blockchain.
    virtual auto Filters() const noexcept -> Position = 0;
    /// Returns the number of block headers downloaded so far for this
    /// blockchain.
    virtual auto Headers() const noexcept -> Position = 0;
    /// Returns the display name for this blockchain.
    virtual auto Name() const noexcept -> UnallocatedCString = 0;

    BlockchainStatisticsItem(const BlockchainStatisticsItem&) = delete;
    BlockchainStatisticsItem(BlockchainStatisticsItem&&) = delete;
    auto operator=(const BlockchainStatisticsItem&)
        -> BlockchainStatisticsItem& = delete;
    auto operator=(BlockchainStatisticsItem&&)
        -> BlockchainStatisticsItem& = delete;

    ~BlockchainStatisticsItem() override = default;

protected:
    BlockchainStatisticsItem() noexcept = default;
};
}  // namespace opentxs::ui
