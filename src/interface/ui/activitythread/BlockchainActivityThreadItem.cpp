// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/ui/activitythread/BlockchainActivityThreadItem.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "interface/ui/activitythread/ActivityThreadItem.hpp"
#include "interface/ui/base/Widget.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"

namespace opentxs::factory
{
auto BlockchainActivityThreadItem(
    const ui::implementation::ActivityThreadInternalInterface& parent,
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const ui::implementation::ActivityThreadRowID& rowID,
    const ui::implementation::ActivityThreadSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::ActivityThreadRowInternal>
{
    using ReturnType = ui::implementation::BlockchainActivityThreadItem;

    auto [txid, amount, display, memo] =
        ReturnType::extract(api, nymID, custom);

    return std::make_shared<ReturnType>(
        parent,
        api,
        nymID,
        rowID,
        sortKey,
        custom,
        std::move(txid),
        amount,
        std::move(display),
        std::move(memo));
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
BlockchainActivityThreadItem::BlockchainActivityThreadItem(
    const ActivityThreadInternalInterface& parent,
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const ActivityThreadRowID& rowID,
    const ActivityThreadSortKey& sortKey,
    CustomData& custom,
    blockchain::block::TransactionHash&& txid,
    opentxs::Amount amount,
    UnallocatedCString&& displayAmount,
    UnallocatedCString&& memo) noexcept
    : ActivityThreadItem(parent, api, nymID, rowID, sortKey, custom)
    , txid_(std::move(txid))
    , display_amount_(std::move(displayAmount))
    , memo_(std::move(memo))
    , amount_(amount)
{
    OT_ASSERT(false == nym_id_.empty());
    OT_ASSERT(false == item_id_.empty());
    OT_ASSERT(false == txid_.empty());
}

auto BlockchainActivityThreadItem::Amount() const noexcept -> opentxs::Amount
{
    auto lock = sLock{shared_lock_};

    return amount_;
}

auto BlockchainActivityThreadItem::DisplayAmount() const noexcept
    -> UnallocatedCString
{
    auto lock = sLock{shared_lock_};

    return display_amount_;
}

auto BlockchainActivityThreadItem::extract(
    const api::session::Client& api,
    const identifier::Nym& nymID,
    CustomData& custom) noexcept
    -> std::tuple<
        blockchain::block::TransactionHash,
        opentxs::Amount,
        UnallocatedCString,
        UnallocatedCString>
{
    return std::tuple<
        blockchain::block::TransactionHash,
        opentxs::Amount,
        UnallocatedCString,
        UnallocatedCString>{
        ui::implementation::extract_custom<UnallocatedCString>(custom, 5),
        ui::implementation::extract_custom<opentxs::Amount>(custom, 6),
        ui::implementation::extract_custom<UnallocatedCString>(custom, 7),
        ui::implementation::extract_custom<UnallocatedCString>(custom, 8)};
}

auto BlockchainActivityThreadItem::Memo() const noexcept -> UnallocatedCString
{
    auto lock = sLock{shared_lock_};

    return memo_;
}

auto BlockchainActivityThreadItem::reindex(
    const ActivityThreadSortKey& key,
    CustomData& custom) noexcept -> bool
{
    auto [txid, amount, display, memo] = extract(api_, nym_id_, custom);
    auto output = ActivityThreadItem::reindex(key, custom);

    OT_ASSERT(txid_ == txid);

    auto lock = eLock{shared_lock_};

    if (display_amount_ != display) {
        display_amount_ = std::move(display);
        output = true;
    }

    if (memo_ != memo) {
        memo_ = std::move(memo);
        output = true;
    }

    if (amount_ != amount) {
        amount_ = amount;
        output = true;
    }

    return output;
}
}  // namespace opentxs::ui::implementation
