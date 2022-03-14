// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/Types.hpp"
#include "opentxs/interface/ui/ListRow.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace ui
{
class BlockchainStatisticsItem;
}  // namespace ui

using OTUIBlockchainStatisticsItem = SharedPimpl<ui::BlockchainStatisticsItem>;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
class OPENTXS_EXPORT BlockchainStatisticsItem : virtual public ListRow
{
public:
    using Position = blockchain::block::Height;

    virtual auto ActivePeers() const noexcept -> std::size_t = 0;
    virtual auto Balance() const noexcept -> UnallocatedCString = 0;
    virtual auto BlockDownloadQueue() const noexcept -> std::size_t = 0;
    virtual auto Chain() const noexcept -> blockchain::Type = 0;
    virtual auto ConnectedPeers() const noexcept -> std::size_t = 0;
    virtual auto Filters() const noexcept -> Position = 0;
    virtual auto Headers() const noexcept -> Position = 0;
    virtual auto Name() const noexcept -> UnallocatedCString = 0;

    ~BlockchainStatisticsItem() override = default;

protected:
    BlockchainStatisticsItem() noexcept = default;

private:
    BlockchainStatisticsItem(const BlockchainStatisticsItem&) = delete;
    BlockchainStatisticsItem(BlockchainStatisticsItem&&) = delete;
    auto operator=(const BlockchainStatisticsItem&)
        -> BlockchainStatisticsItem& = delete;
    auto operator=(BlockchainStatisticsItem&&)
        -> BlockchainStatisticsItem& = delete;
};
}  // namespace opentxs::ui
