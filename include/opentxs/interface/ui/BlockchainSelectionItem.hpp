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
class BlockchainSelectionItem;
}  // namespace ui

using OTUIBlockchainSelectionItem = SharedPimpl<ui::BlockchainSelectionItem>;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
class OPENTXS_EXPORT BlockchainSelectionItem : virtual public ListRow
{
public:
    virtual auto Name() const noexcept -> UnallocatedCString = 0;
    virtual auto IsEnabled() const noexcept -> bool = 0;
    virtual auto IsTestnet() const noexcept -> bool = 0;
    virtual auto Type() const noexcept -> blockchain::Type = 0;

    ~BlockchainSelectionItem() override = default;

protected:
    BlockchainSelectionItem() noexcept = default;

private:
    BlockchainSelectionItem(const BlockchainSelectionItem&) = delete;
    BlockchainSelectionItem(BlockchainSelectionItem&&) = delete;
    auto operator=(const BlockchainSelectionItem&)
        -> BlockchainSelectionItem& = delete;
    auto operator=(BlockchainSelectionItem&&)
        -> BlockchainSelectionItem& = delete;
};
}  // namespace opentxs::ui
