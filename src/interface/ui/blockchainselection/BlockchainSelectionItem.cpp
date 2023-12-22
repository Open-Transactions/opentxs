// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/ui/blockchainselection/BlockchainSelectionItem.hpp"  // IWYU pragma: associated

#include <memory>

#include "interface/ui/base/Widget.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto BlockchainSelectionItem(
    const ui::implementation::BlockchainSelectionInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::BlockchainSelectionRowID& rowID,
    const ui::implementation::BlockchainSelectionSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::BlockchainSelectionRowInternal>
{
    using ReturnType = ui::implementation::BlockchainSelectionItem;

    return std::make_shared<ReturnType>(parent, api, rowID, sortKey, custom);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
BlockchainSelectionItem::BlockchainSelectionItem(
    const BlockchainSelectionInternalInterface& parent,
    const api::session::Client& api,
    const BlockchainSelectionRowID& rowID,
    const BlockchainSelectionSortKey& sortKey,
    CustomData& custom) noexcept
    : BlockchainSelectionItemRow(parent, api, rowID, true)
    , api_(api)
    , testnet_(sortKey.second)
    , name_(sortKey.first)
    , enabled_(extract_custom<bool>(custom, 0))
{
}

auto BlockchainSelectionItem::reindex(
    const BlockchainSelectionSortKey& key,
    CustomData& custom) noexcept -> bool
{
    assert_true(testnet_ == key.second);
    assert_true(name_ == key.first);

    const auto lock = Lock{lock_};

    if (auto enabled = extract_custom<bool>(custom, 0); enabled_ != enabled) {
        enabled_ = enabled;

        return true;
    }

    return false;
}

BlockchainSelectionItem::~BlockchainSelectionItem() = default;
}  // namespace opentxs::ui::implementation
