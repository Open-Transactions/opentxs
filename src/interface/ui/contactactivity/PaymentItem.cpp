// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/ui/contactactivity/PaymentItem.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "interface/ui/contactactivity/ContactActivityItem.hpp"
#include "internal/api/session/Activity.hpp"
#include "internal/core/String.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/otx/client/OTPayment.hpp"
#include "internal/otx/common/Cheque.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/Pimpl.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/session/Activity.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Factory.internal.hpp"
#include "opentxs/api/session/OTX.hpp"
#include "opentxs/display/Definition.hpp"
#include "opentxs/identifier/Account.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/PasswordPrompt.hpp"  // IWYU pragma: keep

namespace opentxs::factory
{
auto PaymentItem(
    const ui::implementation::ContactActivityInternalInterface& parent,
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const ui::implementation::ContactActivityRowID& rowID,
    const ui::implementation::ContactActivitySortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::ContactActivityRowInternal>
{
    using ReturnType = ui::implementation::PaymentItem;
    auto [amount, display, memo, contract] =
        ReturnType::extract(api, nymID, rowID, custom);

    return std::make_shared<ReturnType>(
        parent,
        api,
        nymID,
        rowID,
        sortKey,
        custom,
        amount,
        std::move(display),
        std::move(memo),
        std::move(contract));
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
PaymentItem::PaymentItem(
    const ContactActivityInternalInterface& parent,
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const ContactActivityRowID& rowID,
    const ContactActivitySortKey& sortKey,
    CustomData& custom,
    opentxs::Amount amount,
    UnallocatedCString&& display,
    UnallocatedCString&& memo,
    std::shared_ptr<const OTPayment>&& contract) noexcept
    : ContactActivityItem(parent, api, nymID, rowID, sortKey, custom)
    , amount_(amount)
    , display_amount_(std::move(display))
    , memo_(std::move(memo))
    , payment_(std::move(contract))
{
    assert_false(nym_id_.empty());
    assert_false(item_id_.empty());
}

auto PaymentItem::Amount() const noexcept -> opentxs::Amount
{
    auto lock = sLock{shared_lock_};

    return amount_;
}

auto PaymentItem::Deposit() const noexcept -> bool
{
    switch (box_) {
        case otx::client::StorageBox::INCOMINGCHEQUE: {
        } break;
        case otx::client::StorageBox::OUTGOINGCHEQUE:
        case otx::client::StorageBox::SENTPEERREQUEST:
        case otx::client::StorageBox::INCOMINGPEERREQUEST:
        case otx::client::StorageBox::SENTPEERREPLY:
        case otx::client::StorageBox::INCOMINGPEERREPLY:
        case otx::client::StorageBox::FINISHEDPEERREQUEST:
        case otx::client::StorageBox::FINISHEDPEERREPLY:
        case otx::client::StorageBox::PROCESSEDPEERREQUEST:
        case otx::client::StorageBox::PROCESSEDPEERREPLY:
        case otx::client::StorageBox::MAILINBOX:
        case otx::client::StorageBox::MAILOUTBOX:
        case otx::client::StorageBox::BLOCKCHAIN:
        case otx::client::StorageBox::DRAFT:
        case otx::client::StorageBox::UNKNOWN:
        default: {

            return false;
        }
    }

    auto lock = sLock{shared_lock_};

    if (false == bool(payment_)) {
        LogError()()("Payment not loaded.").Flush();

        return false;
    }

    auto task = api_.OTX().DepositPayment(nym_id_, payment_);

    if (0 == task.first) {
        LogError()()("Failed to queue deposit.").Flush();

        return false;
    }

    return true;
}

auto PaymentItem::DisplayAmount() const noexcept -> UnallocatedCString
{
    auto lock = sLock{shared_lock_};

    return display_amount_;
}

auto PaymentItem::extract(
    const api::session::Client& api,
    const identifier::Nym& nym,
    const ContactActivityRowID& row,
    CustomData& custom) noexcept
    -> std::tuple<
        opentxs::Amount,
        UnallocatedCString,
        UnallocatedCString,
        std::shared_ptr<const OTPayment>>
{
    const auto reason =
        api.Factory().PasswordPrompt("Decrypting payment activity");
    const auto& [itemID, box, account] = row;
    auto& text = *static_cast<UnallocatedCString*>(custom.front());
    auto output = std::tuple<
        opentxs::Amount,
        UnallocatedCString,
        UnallocatedCString,
        std::shared_ptr<OTPayment>>{};
    auto& [amount, displayAmount, memo, payment] = output;

    switch (box) {
        case otx::client::StorageBox::INCOMINGCHEQUE:
        case otx::client::StorageBox::OUTGOINGCHEQUE: {
            auto message = api.Activity().PaymentText(
                nym, itemID.asBase58(api.Crypto()), account);

            if (message) { text = *message; }

            const auto [cheque, contract] =
                api.Activity().Internal().Cheque(nym, account);

            if (cheque) {
                memo = cheque->GetMemo().Get();
                amount = cheque->GetAmount();

                if (0 < contract->Version()) {
                    const auto& definition =
                        display::GetDefinition(contract->UnitOfAccount());
                    displayAmount = definition.Format(amount);
                }

                payment = api.Factory().Internal().Session().Payment(
                    String::Factory(*cheque));

                assert_false(nullptr == payment);

                payment->SetTempValues(reason);
            }
        } break;
        case otx::client::StorageBox::SENTPEERREQUEST:
        case otx::client::StorageBox::INCOMINGPEERREQUEST:
        case otx::client::StorageBox::SENTPEERREPLY:
        case otx::client::StorageBox::INCOMINGPEERREPLY:
        case otx::client::StorageBox::FINISHEDPEERREQUEST:
        case otx::client::StorageBox::FINISHEDPEERREPLY:
        case otx::client::StorageBox::PROCESSEDPEERREQUEST:
        case otx::client::StorageBox::PROCESSEDPEERREPLY:
        case otx::client::StorageBox::MAILINBOX:
        case otx::client::StorageBox::MAILOUTBOX:
        case otx::client::StorageBox::BLOCKCHAIN:
        case otx::client::StorageBox::DRAFT:
        case otx::client::StorageBox::UNKNOWN:
        default: {
            LogAbort()().Abort();
        }
    }

    return output;
}

auto PaymentItem::Memo() const noexcept -> UnallocatedCString
{
    auto lock = sLock{shared_lock_};

    return memo_;
}

auto PaymentItem::reindex(
    const ContactActivitySortKey& key,
    CustomData& custom) noexcept -> bool
{
    auto [amount, display, memo, contract] =
        extract(api_, nym_id_, row_id_, custom);
    auto changed = ContactActivityItem::reindex(key, custom);
    auto lock = eLock{shared_lock_};

    if (amount_ != amount) {
        amount_ = amount;
        changed = true;
    }

    if (display_amount_ != display) {
        display_amount_ = std::move(display);
        changed = true;
    }

    if (memo_ != memo) {
        memo_ = std::move(memo);
        changed = true;
    }

    if (contract && (!payment_)) {
        payment_ = std::move(contract);
        changed = true;
    }

    return changed;
}

PaymentItem::~PaymentItem() = default;
}  // namespace opentxs::ui::implementation
