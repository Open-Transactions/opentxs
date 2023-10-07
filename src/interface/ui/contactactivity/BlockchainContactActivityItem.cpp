// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/ui/contactactivity/BlockchainContactActivityItem.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "interface/ui/base/Widget.hpp"
#include "interface/ui/contactactivity/ContactActivityItem.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto BlockchainContactActivityItem(
    const ui::implementation::ContactActivityInternalInterface& parent,
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const ui::implementation::ContactActivityRowID& rowID,
    const ui::implementation::ContactActivitySortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::ContactActivityRowInternal>
{
    using ReturnType = ui::implementation::BlockchainContactActivityItem;

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
BlockchainContactActivityItem::BlockchainContactActivityItem(
    const ContactActivityInternalInterface& parent,
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const ContactActivityRowID& rowID,
    const ContactActivitySortKey& sortKey,
    CustomData& custom,
    blockchain::block::TransactionHash&& txid,
    opentxs::Amount amount,
    UnallocatedCString&& displayAmount,
    UnallocatedCString&& memo) noexcept
    : ContactActivityItem(parent, api, nymID, rowID, sortKey, custom)
    , txid_(std::move(txid))
    , display_amount_(std::move(displayAmount))
    , memo_(std::move(memo))
    , amount_(amount)
{
    assert_false(nym_id_.empty());
    assert_false(item_id_.empty());
    assert_false(txid_.empty());
}

auto BlockchainContactActivityItem::Amount() const noexcept -> opentxs::Amount
{
    auto lock = sLock{shared_lock_};

    return amount_;
}

auto BlockchainContactActivityItem::DisplayAmount() const noexcept
    -> UnallocatedCString
{
    auto lock = sLock{shared_lock_};

    return display_amount_;
}

auto BlockchainContactActivityItem::extract(
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

auto BlockchainContactActivityItem::Memo() const noexcept -> UnallocatedCString
{
    auto lock = sLock{shared_lock_};

    return memo_;
}

auto BlockchainContactActivityItem::reindex(
    const ContactActivitySortKey& key,
    CustomData& custom) noexcept -> bool
{
    auto [txid, amount, display, memo] = extract(api_, nym_id_, custom);
    auto output = ContactActivityItem::reindex(key, custom);

    assert_true(txid_ == txid);

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

auto BlockchainContactActivityItem::TXID() const noexcept -> UnallocatedCString
{
    auto lock = sLock{shared_lock_};

    return blockchain::HashToNumber(txid_);
}
}  // namespace opentxs::ui::implementation
