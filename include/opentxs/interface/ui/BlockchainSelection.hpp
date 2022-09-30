// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/interface/ui/List.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
class BlockchainSelection;
class BlockchainSelectionItem;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
/**
  The rows in this model each represent a blockchain supported by the wallet.
  Each chain can be individually enabled or disabled.
*/
class OPENTXS_EXPORT BlockchainSelection : virtual public List
{
public:
    /// returns the first row, containing a valid BlockchainSelectionItem or an
    /// empty smart pointer (if list is empty).
    virtual auto First() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::BlockchainSelectionItem> = 0;
    /// returns the next row, containing a valid BlockchainSelectionItem or an
    /// empty smart pointer (if at end of list).
    virtual auto Next() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::BlockchainSelectionItem> = 0;

    /// This function can be used to disable a blockchain in the wallet. Returns
    /// success or failure.
    virtual auto Disable(const blockchain::Type type) const noexcept
        -> bool = 0;
    /// This function can be used to enable a blockchain in the wallet. Returns
    /// success or failure.
    virtual auto Enable(const blockchain::Type type) const noexcept -> bool = 0;

    BlockchainSelection(const BlockchainSelection&) = delete;
    BlockchainSelection(BlockchainSelection&&) = delete;
    auto operator=(const BlockchainSelection&) -> BlockchainSelection& = delete;
    auto operator=(BlockchainSelection&&) -> BlockchainSelection& = delete;

    ~BlockchainSelection() override = default;

protected:
    BlockchainSelection() noexcept = default;
};
}  // namespace opentxs::ui
