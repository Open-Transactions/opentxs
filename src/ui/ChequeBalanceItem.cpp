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
#include "ChequeBalanceItem.hpp"

#define OT_METHOD "opentxs::ui::implementation::ChequeBalanceItem::"

namespace opentxs::ui::implementation
{
ChequeBalanceItem::ChequeBalanceItem(
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
    , cheque_(nullptr)
{
    startup_.reset(new std::thread(&ChequeBalanceItem::startup, this, custom));

    OT_ASSERT(startup_)
}

opentxs::Amount ChequeBalanceItem::effective_amount() const
{
    sLock lock(shared_lock_);
    auto amount{0};
    opentxs::Amount sign{0};

    if (cheque_) { amount = cheque_->GetAmount(); }

    switch (type_) {
        case StorageBox::OUTGOINGCHEQUE: {
            sign = -1;
        } break;
        case StorageBox::INCOMINGCHEQUE: {
            sign = 1;
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

    return amount * sign;
}

bool ChequeBalanceItem::get_contract() const
{
    if (contract_) { return true; }

    eLock lock(shared_lock_);
    const auto& contractID = cheque_->GetInstrumentDefinitionID();
    contract_ = api_.Wallet().UnitDefinition(contractID);

    if (contract_) { return true; }

    api_.OTX().DownloadContract(
        nym_id_, api_.OTX().IntroductionServer(), contractID);

    return false;
}

std::string ChequeBalanceItem::Memo() const
{
    sLock lock(shared_lock_);

    if (cheque_) { return cheque_->GetMemo().Get(); }

    return {};
}

void ChequeBalanceItem::reindex(
    const implementation::AccountActivitySortKey& key,
    const implementation::CustomData& custom)
{
    BalanceItem::reindex(key, custom);
    startup(custom);
}

void ChequeBalanceItem::startup(const CustomData& custom)
{
    OT_ASSERT(2 == custom.size())

    const auto workflow = extract_custom<proto::PaymentWorkflow>(custom, 0);
    const auto event = extract_custom<proto::PaymentEvent>(custom, 1);
    eLock lock(shared_lock_);
    cheque_ = api::client::Workflow::InstantiateCheque(api_, workflow).second;

    OT_ASSERT(cheque_)

    lock.unlock();
    get_contract();
    std::string name{""};
    std::string text{""};
    auto number = std::to_string(cheque_->GetTransactionNum());
    auto otherNymID = identifier::Nym::Factory();

    switch (type_) {
        case StorageBox::INCOMINGCHEQUE: {
            otherNymID =
                identifier::Nym::Factory(cheque_->GetSenderNymID().str());

            if (otherNymID->empty()) { otherNymID = nym_id_; }

            switch (event.type()) {
                case proto::PAYMENTEVENTTYPE_CONVEY: {
                    text = "Received cheque #" + number + " from " +
                           get_contact_name(otherNymID);
                } break;
                case proto::PAYMENTEVENTTYPE_ERROR:
                case proto::PAYMENTEVENTTYPE_CREATE:
                case proto::PAYMENTEVENTTYPE_ACCEPT:
                case proto::PAYMENTEVENTTYPE_CANCEL:
                case proto::PAYMENTEVENTTYPE_COMPLETE:
                default: {
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Invalid event state (")(event.type())(")")
                        .Flush();
                }
            }
        } break;
        case StorageBox::OUTGOINGCHEQUE: {
            otherNymID =
                identifier::Nym::Factory(cheque_->GetRecipientNymID().str());

            switch (event.type()) {
                case proto::PAYMENTEVENTTYPE_CREATE: {
                    text = "Wrote cheque #" + number;

                    if (false == otherNymID->empty()) {
                        text += " for " + get_contact_name(otherNymID);
                    }
                } break;
                case proto::PAYMENTEVENTTYPE_ACCEPT: {
                    text = "Cheque #" + number + " cleared";
                } break;
                case proto::PAYMENTEVENTTYPE_ERROR:
                case proto::PAYMENTEVENTTYPE_CONVEY:
                case proto::PAYMENTEVENTTYPE_CANCEL:
                case proto::PAYMENTEVENTTYPE_COMPLETE:
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
        case StorageBox::OUTGOINGTRANSFER:
        case StorageBox::INCOMINGTRANSFER:
        case StorageBox::INTERNALTRANSFER:
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

std::string ChequeBalanceItem::UUID() const
{
    if (cheque_) {

        return api::client::Workflow::UUID(
                   cheque_->GetNotaryID(), cheque_->GetTransactionNum())
            ->str();
    }

    return {};
}
}  // namespace opentxs::ui::implementation
