// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/OTX.hpp"
#include "opentxs/api/client/Workflow.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/ui/BalanceItem.hpp"

#include "InternalUI.hpp"
#include "Row.hpp"

#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

#include "BalanceItem.hpp"
#include "TransferBalanceItem.hpp"

#define OT_METHOD "opentxs::ui::implementation::TransferBalanceItem::"

namespace opentxs::ui::implementation
{
TransferBalanceItem::TransferBalanceItem(
    const AccountActivityInternalInterface& parent,
    const api::client::Manager& api,
    const network::zeromq::PublishSocket& publisher,
    const AccountActivityRowID& rowID,
    const AccountActivitySortKey& sortKey,
    const CustomData& custom,
    const identifier::Nym& nymID,
    const Identifier& accountID)
    : BalanceItem(
          parent,
          api,
          publisher,
          rowID,
          sortKey,
          custom,
          nymID,
          accountID)
{
    startup_.reset(
        new std::thread(&TransferBalanceItem::startup, this, custom));

    OT_ASSERT(startup_)
}

opentxs::Amount TransferBalanceItem::effective_amount() const
{
    sLock lock(shared_lock_);
    auto amount{0};
    opentxs::Amount sign{0};

    if (transfer_) { amount = transfer_->GetAmount(); }

    switch (type_) {
        case StorageBox::OUTGOINGTRANSFER: {
            sign = -1;
        } break;
        case StorageBox::INCOMINGTRANSFER: {
            sign = 1;
        } break;
        case StorageBox::INTERNALTRANSFER: {
            const auto in =
                parent_.AccountID() == transfer_->GetDestinationAcctID();

            if (in) {
                sign = 1;
            } else {
                sign = -1;
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
        case StorageBox::INCOMINGCHEQUE:
        case StorageBox::OUTGOINGCHEQUE:
        case StorageBox::DRAFT:
        case StorageBox::UNKNOWN:
        default: {
        }
    }

    return amount * sign;
}

bool TransferBalanceItem::get_contract() const
{
    if (contract_) { return true; }

    auto contractID = Identifier::Factory();
    const auto in = parent_.AccountID() == transfer_->GetDestinationAcctID();

    if (in) {
        contractID =
            api_.Storage().AccountContract(transfer_->GetDestinationAcctID());
    } else {
        contractID =
            api_.Storage().AccountContract(transfer_->GetPurportedAccountID());
    }

    eLock lock(shared_lock_);
    contract_ = api_.Wallet().UnitDefinition(contractID);
    lock.unlock();

    if (contract_) { return true; }

    api_.OTX().DownloadContract(
        nym_id_, api_.OTX().IntroductionServer(), contractID);

    return false;
}

std::string TransferBalanceItem::Memo() const
{
    sLock lock(shared_lock_);

    if (transfer_) {
        auto note = String::Factory();
        transfer_->GetNote(note);

        return note->Get();
    }

    return {};
}

void TransferBalanceItem::reindex(
    const implementation::AccountActivitySortKey& key,
    const implementation::CustomData& custom)
{
    BalanceItem::reindex(key, custom);
    startup(custom);
}

void TransferBalanceItem::startup(const CustomData& custom)
{
    OT_ASSERT(2 == custom.size())

    const auto workflow = extract_custom<proto::PaymentWorkflow>(custom, 0);
    const auto event = extract_custom<proto::PaymentEvent>(custom, 1);
    eLock lock(shared_lock_);
    transfer_ =
        api::client::Workflow::InstantiateTransfer(api_, workflow).second;

    OT_ASSERT(transfer_)

    lock.unlock();
    get_contract();
    std::string text{""};
    const auto number = std::to_string(transfer_->GetTransactionNum());

    switch (type_) {
        case StorageBox::OUTGOINGTRANSFER: {
            switch (event.type()) {
                case proto::PAYMENTEVENTTYPE_ACKNOWLEDGE: {
                    text = "Sent transfer #" + number + " to ";

                    if (0 < workflow.party_size()) {
                        text += get_contact_name(
                            identifier::Nym::Factory(workflow.party(0)));
                    } else {
                        text += "account " +
                                transfer_->GetDestinationAcctID().str();
                    }
                } break;
                case proto::PAYMENTEVENTTYPE_COMPLETE: {
                    text = "Transfer #" + number + " cleared.";
                } break;
                default: {
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Invalid event state (")(event.type())(")")
                        .Flush();
                }
            }
        } break;
        case StorageBox::INCOMINGTRANSFER: {
            switch (event.type()) {
                case proto::PAYMENTEVENTTYPE_CONVEY: {
                    text = "Received transfer #" + number + " from ";

                    if (0 < workflow.party_size()) {
                        text += get_contact_name(
                            identifier::Nym::Factory(workflow.party(0)));
                    } else {
                        text += "account " +
                                transfer_->GetPurportedAccountID().str();
                    }
                } break;
                case proto::PAYMENTEVENTTYPE_COMPLETE: {
                    text = "Transfer #" + number + " cleared.";
                } break;
                default: {
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Invalid event state (")(event.type())(")")
                        .Flush();
                }
            }
        } break;
        case StorageBox::INTERNALTRANSFER: {
            const auto in =
                parent_.AccountID() == transfer_->GetDestinationAcctID();

            switch (event.type()) {
                case proto::PAYMENTEVENTTYPE_ACKNOWLEDGE: {
                    if (in) {
                        text = "Received internal transfer #" + number +
                               " from account " +
                               transfer_->GetPurportedAccountID().str();
                    } else {
                        text = "Sent internal transfer #" + number +
                               " to account " +
                               transfer_->GetDestinationAcctID().str();
                    }
                } break;
                case proto::PAYMENTEVENTTYPE_COMPLETE: {
                    text = "Transfer #" + number + " cleared.";
                } break;
                default: {
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Invalid event state (")(event.type())(")")
                        .Flush();
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
        case StorageBox::INCOMINGCHEQUE:
        case StorageBox::OUTGOINGCHEQUE:
        case StorageBox::DRAFT:
        case StorageBox::UNKNOWN:
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid item type (")(
                static_cast<std::uint8_t>(type_))(")")
                .Flush();
        }
    }

    lock.lock();
    text_ = text;
    lock.unlock();
    UpdateNotify();
}

std::string TransferBalanceItem::UUID() const
{
    if (transfer_) {

        return api::client::Workflow::UUID(
                   transfer_->GetPurportedNotaryID(),
                   transfer_->GetTransactionNum())
            ->str();
    }

    return {};
}
}  // namespace opentxs::ui::implementation
