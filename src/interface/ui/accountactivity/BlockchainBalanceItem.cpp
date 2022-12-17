// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/ui/accountactivity/BlockchainBalanceItem.hpp"  // IWYU pragma: associated

#include <PaymentEvent.pb.h>
#include <PaymentWorkflow.pb.h>
#include <memory>
#include <utility>

#include "interface/ui/accountactivity/BalanceItem.hpp"
#include "interface/ui/base/Widget.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/bitcoin/block/Transaction.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::factory
{
auto BalanceItemBlockchain(
    const ui::implementation::AccountActivityInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::AccountActivityRowID& rowID,
    const ui::implementation::AccountActivitySortKey& sortKey,
    ui::implementation::CustomData& custom,
    const identifier::Nym& nymID,
    const identifier::Generic& accountID) noexcept
    -> std::shared_ptr<ui::implementation::AccountActivityRowInternal>
{
    using Transaction = opentxs::blockchain::block::Transaction;

    auto tx =
        std::move(ui::implementation::extract_custom<Transaction>(custom, 2))
            .asBitcoin();

    return std::make_shared<ui::implementation::BlockchainBalanceItem>(
        parent,
        api,
        rowID,
        sortKey,
        custom,
        nymID,
        accountID,
        ui::implementation::extract_custom<blockchain::Type>(custom, 3),
        blockchain::block::TransactionHash{
            ui::implementation::extract_custom<UnallocatedCString>(custom, 5)},
        tx.NetBalanceChange(api.Crypto().Blockchain(), nymID),
        UnallocatedCString{tx.Memo(api.Crypto().Blockchain())},
        ui::implementation::extract_custom<UnallocatedCString>(custom, 4));
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
BlockchainBalanceItem::BlockchainBalanceItem(
    const AccountActivityInternalInterface& parent,
    const api::session::Client& api,
    const AccountActivityRowID& rowID,
    const AccountActivitySortKey& sortKey,
    CustomData& custom,
    const identifier::Nym& nymID,
    const identifier::Generic& accountID,
    [[maybe_unused]] const blockchain::Type chain,
    const blockchain::block::TransactionHash& txid,
    const opentxs::Amount amount,
    const UnallocatedCString memo,
    const UnallocatedCString text) noexcept
    : BalanceItem(parent, api, rowID, sortKey, custom, nymID, accountID, text)
    , chain_(chain)
    , txid_(txid)
    , amount_(amount)
    , memo_(memo)
    , confirmations_(extract_custom<int>(custom, 6))
{
    // NOTE Avoids memory leaks
    extract_custom<proto::PaymentWorkflow>(custom, 0);
    extract_custom<proto::PaymentEvent>(custom, 1);
}

auto BlockchainBalanceItem::Contacts() const noexcept
    -> UnallocatedVector<UnallocatedCString>
{
    auto output = UnallocatedVector<UnallocatedCString>{};

    for (const auto& id : api_.Storage().BlockchainThreadMap(nym_id_, txid_)) {
        if (0 < id.size()) { output.emplace_back(id.asBase58(api_.Crypto())); }
    }

    return output;
}

auto BlockchainBalanceItem::DisplayAmount() const noexcept -> UnallocatedCString
{
    sLock lock(shared_lock_);

    return blockchain::internal::Format(chain_, amount_);
}

auto BlockchainBalanceItem::Memo() const noexcept -> UnallocatedCString
{
    sLock lock{shared_lock_};

    return memo_;
}

auto BlockchainBalanceItem::reindex(
    const implementation::AccountActivitySortKey& key,
    implementation::CustomData& custom) noexcept -> bool
{
    using Transaction = opentxs::blockchain::block::Transaction;

    extract_custom<proto::PaymentWorkflow>(custom, 0);
    extract_custom<proto::PaymentEvent>(custom, 1);
    const auto tx =
        std::move(extract_custom<Transaction>(custom, 2)).asBitcoin();
    auto output = BalanceItem::reindex(key, custom);
    const auto chain = extract_custom<blockchain::Type>(custom, 3);
    const auto txid = blockchain::block::TransactionHash{
        ui::implementation::extract_custom<UnallocatedCString>(custom, 5)};
    const auto amount =
        tx.NetBalanceChange(api_.Crypto().Blockchain(), nym_id_);
    const auto memo = tx.Memo(api_.Crypto().Blockchain());
    const auto text = extract_custom<UnallocatedCString>(custom, 4);
    const auto conf = extract_custom<int>(custom, 6);

    OT_ASSERT(chain_ == chain);
    OT_ASSERT(txid_ == txid);

    eLock lock{shared_lock_};
    const auto oldAmount = amount_;
    amount_ = amount;

    if (oldAmount != amount) { output |= true; }

    if (memo_ != memo) {
        memo_ = memo;
        output |= true;
    }

    if (text_ != text) {
        text_ = text;
        output |= true;
    }

    if (auto previous = confirmations_.exchange(conf); previous != conf) {
        output |= true;
    }

    return output;
}

auto BlockchainBalanceItem::UUID() const noexcept -> UnallocatedCString
{
    return blockchain::HashToNumber(txid_);
}
}  // namespace opentxs::ui::implementation
