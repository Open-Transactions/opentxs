// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Activity.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/ui/ActivityThreadItem.hpp"

#include "InternalUI.hpp"
#include "Row.hpp"

#include <memory>
#include <thread>

#include "PaymentItem.hpp"

namespace opentxs
{
ui::implementation::ActivityThreadRowInternal* Factory::PaymentItem(
    const ui::implementation::ActivityThreadInternalInterface& parent,
    const api::client::Manager& api,
    const network::zeromq::PublishSocket& publisher,
    const identifier::Nym& nymID,
    const ui::implementation::ActivityThreadRowID& rowID,
    const ui::implementation::ActivityThreadSortKey& sortKey,
    const ui::implementation::CustomData& custom)
{
    return new ui::implementation::PaymentItem(
        parent, api, publisher, nymID, rowID, sortKey, custom);
}
}  // namespace opentxs

namespace opentxs::ui::implementation
{
PaymentItem::PaymentItem(
    const ActivityThreadInternalInterface& parent,
    const api::client::Manager& api,
    const network::zeromq::PublishSocket& publisher,
    const identifier::Nym& nymID,
    const ActivityThreadRowID& rowID,
    const ActivityThreadSortKey& sortKey,
    const CustomData& custom)
    : ActivityThreadItem(
          parent,
          api,
          publisher,
          nymID,
          rowID,
          sortKey,
          custom,
          true,
          false)
    , display_amount_()
    , memo_()
    , amount_(0)
    , load_(nullptr)
{
    OT_ASSERT(false == nym_id_.empty())
    OT_ASSERT(false == item_id_.empty())

    switch (box_) {
        case StorageBox::INCOMINGCHEQUE:
        case StorageBox::OUTGOINGCHEQUE: {
            load_.reset(new std::thread(&PaymentItem::load, this));
        } break;
        case StorageBox::SENTPEERREQUEST:
        case StorageBox::INCOMINGPEERREQUEST:
        case StorageBox::SENTPEERREPLY:
        case StorageBox::INCOMINGPEERREPLY:
        case StorageBox::FINISHEDPEERREQUEST:
        case StorageBox::FINISHEDPEERREPLY:
        case StorageBox::PROCESSEDPEERREQUEST:
        case StorageBox::PROCESSEDPEERREPLY:
        case StorageBox::MAILINBOX:
        case StorageBox::MAILOUTBOX:
        case StorageBox::INCOMINGBLOCKCHAIN:
        case StorageBox::OUTGOINGBLOCKCHAIN:
        case StorageBox::DRAFT:
        case StorageBox::UNKNOWN:
        default: {
        }
    }

    OT_ASSERT(load_)
}

opentxs::Amount PaymentItem::Amount() const
{
    sLock lock(shared_lock_);

    return amount_;
}

std::string PaymentItem::DisplayAmount() const
{
    sLock lock(shared_lock_);

    return display_amount_;
}

void PaymentItem::load()
{
    std::shared_ptr<const std::string> text{nullptr};
    std::string displayAmount{};
    std::string memo{};
    opentxs::Amount amount{0};

    switch (box_) {
        case StorageBox::INCOMINGCHEQUE:
        case StorageBox::OUTGOINGCHEQUE: {
            text = api_.Activity().PaymentText(
                nym_id_, item_id_.str(), account_id_.str());
            const auto [cheque, contract] = api_.Activity().Cheque(
                nym_id_, item_id_.str(), account_id_.str());

            if (cheque) {
                memo = cheque->GetMemo().Get();
                amount = cheque->GetAmount();

                if (contract) {
                    contract->FormatAmountLocale(
                        amount, displayAmount, ",", ".");
                }
            }
        } break;
        case StorageBox::SENTPEERREQUEST:
        case StorageBox::INCOMINGPEERREQUEST:
        case StorageBox::SENTPEERREPLY:
        case StorageBox::INCOMINGPEERREPLY:
        case StorageBox::FINISHEDPEERREQUEST:
        case StorageBox::FINISHEDPEERREPLY:
        case StorageBox::PROCESSEDPEERREQUEST:
        case StorageBox::PROCESSEDPEERREPLY:
        case StorageBox::MAILINBOX:
        case StorageBox::MAILOUTBOX:
        case StorageBox::INCOMINGBLOCKCHAIN:
        case StorageBox::OUTGOINGBLOCKCHAIN:
        case StorageBox::DRAFT:
        case StorageBox::UNKNOWN:
        default: {
            OT_FAIL
        }
    }

    OT_ASSERT(text)

    eLock lock(shared_lock_);
    text_ = *text;
    display_amount_ = displayAmount;
    memo_ = memo;
    amount_ = amount;
    loading_->Off();
    pending_->Off();
    UpdateNotify();
}

std::string PaymentItem::Memo() const
{
    sLock lock(shared_lock_);

    return memo_;
}

PaymentItem::~PaymentItem()
{
    if (load_ && load_->joinable()) { load_->join(); }
}
}  // namespace opentxs::ui::implementation
