// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/interface/ui/ListRow.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
class BlockchainSelectionItem;
}  // namespace ui

using OTUIBlockchainSelectionItem = SharedPimpl<ui::BlockchainSelectionItem>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
/**
 This model represents a single blockchain. It is a single row from the
 BlockchainSelection model.
*/
class OPENTXS_EXPORT BlockchainSelectionItem : virtual public ListRow
{
public:
    /// Returns the display name of this blockchain.
    virtual auto Name() const noexcept -> UnallocatedCString = 0;
    /// Returns boolean indicating whether or not this blockchain is enabled.
    virtual auto IsEnabled() const noexcept -> bool = 0;
    /// Returns boolean indicating whether or not this blockchain is a testnet.
    virtual auto IsTestnet() const noexcept -> bool = 0;
    /// Returns enum containing the blockchain type.
    virtual auto Type() const noexcept -> blockchain::Type = 0;

    BlockchainSelectionItem(const BlockchainSelectionItem&) = delete;
    BlockchainSelectionItem(BlockchainSelectionItem&&) = delete;
    auto operator=(const BlockchainSelectionItem&)
        -> BlockchainSelectionItem& = delete;
    auto operator=(BlockchainSelectionItem&&)
        -> BlockchainSelectionItem& = delete;

    ~BlockchainSelectionItem() override = default;

protected:
    BlockchainSelectionItem() noexcept = default;
};
}  // namespace opentxs::ui
