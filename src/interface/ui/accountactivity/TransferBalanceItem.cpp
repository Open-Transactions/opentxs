// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/ui/accountactivity/TransferBalanceItem.hpp"  // IWYU pragma: associated

#include <PaymentEvent.pb.h>
#include <PaymentWorkflow.pb.h>
#include <PaymentWorkflowEnums.pb.h>
#include <cstdint>
#include <memory>

#include "interface/ui/accountactivity/BalanceItem.hpp"
#include "interface/ui/base/Widget.hpp"
#include "internal/core/String.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Workflow.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::ui::implementation
{
TransferBalanceItem::TransferBalanceItem(
    const AccountActivityInternalInterface& parent,
    const api::session::Client& api,
    const AccountActivityRowID& rowID,
    const AccountActivitySortKey& sortKey,
    CustomData& custom,
    const identifier::Nym& nymID,
    const identifier::Generic& accountID) noexcept
    : BalanceItem(parent, api, rowID, sortKey, custom, nymID, accountID)
    , transfer_()
{
    startup(
        extract_custom<proto::PaymentWorkflow>(custom, 0),
        extract_custom<proto::PaymentEvent>(custom, 1));
}

auto TransferBalanceItem::effective_amount() const noexcept -> opentxs::Amount
{
    sLock lock(shared_lock_);
    auto amount = opentxs::Amount{0};
    auto sign = opentxs::Amount{0};

    if (transfer_) { amount = transfer_->GetAmount(); }

    switch (type_) {
        case otx::client::StorageBox::OUTGOINGTRANSFER: {
            sign = -1;
        } break;
        case otx::client::StorageBox::INCOMINGTRANSFER: {
            sign = 1;
        } break;
        case otx::client::StorageBox::INTERNALTRANSFER: {
            const auto in =
                parent_.AccountID() ==
                transfer_->GetDestinationAcctID().asBase58(api_.Crypto());

            if (in) {
                sign = 1;
            } else {
                sign = -1;
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
        case otx::client::StorageBox::INCOMINGCHEQUE:
        case otx::client::StorageBox::OUTGOINGCHEQUE:
        case otx::client::StorageBox::DRAFT:
        case otx::client::StorageBox::UNKNOWN:
        case otx::client::StorageBox::RESERVED_1:
        case otx::client::StorageBox::PENDING_SEND:
        default: {
        }
    }

    return amount * sign;
}

auto TransferBalanceItem::Memo() const noexcept -> UnallocatedCString
{
    sLock lock(shared_lock_);

    if (transfer_) {
        auto note = String::Factory();
        transfer_->GetNote(note);

        return note->Get();
    }

    return {};
}

auto TransferBalanceItem::reindex(
    const implementation::AccountActivitySortKey& key,
    implementation::CustomData& custom) noexcept -> bool
{
    auto output = BalanceItem::reindex(key, custom);
    output |= startup(
        extract_custom<proto::PaymentWorkflow>(custom, 0),
        extract_custom<proto::PaymentEvent>(custom, 1));

    return output;
}

auto TransferBalanceItem::startup(
    const proto::PaymentWorkflow workflow,
    const proto::PaymentEvent event) noexcept -> bool
{
    eLock lock(shared_lock_);

    if (false == bool(transfer_)) {
        transfer_ =
            api::session::Workflow::InstantiateTransfer(api_, workflow).second;
    }

    OT_ASSERT(transfer_);

    lock.unlock();
    auto text = UnallocatedCString{};
    const auto number = std::to_string(transfer_->GetTransactionNum());

    switch (type_) {
        case otx::client::StorageBox::OUTGOINGTRANSFER: {
            switch (event.type()) {
                case proto::PAYMENTEVENTTYPE_ACKNOWLEDGE: {
                    text = "Sent transfer #" + number + " to ";

                    if (0 < workflow.party_size()) {
                        text += get_contact_name(
                            api_.Factory().NymIDFromBase58(workflow.party(0)));
                    } else {
                        text += "account " +
                                transfer_->GetDestinationAcctID().asBase58(
                                    api_.Crypto());
                    }
                } break;
                case proto::PAYMENTEVENTTYPE_COMPLETE: {
                    text = "Transfer #" + number + " cleared.";
                } break;
                default: {
                    LogError()(OT_PRETTY_CLASS())("Invalid event state (")(
                        event.type())(")")
                        .Flush();
                }
            }
        } break;
        case otx::client::StorageBox::INCOMINGTRANSFER: {
            switch (event.type()) {
                case proto::PAYMENTEVENTTYPE_CONVEY: {
                    text = "Received transfer #" + number + " from ";

                    if (0 < workflow.party_size()) {
                        text += get_contact_name(
                            api_.Factory().NymIDFromBase58(workflow.party(0)));
                    } else {
                        text += "account " +
                                transfer_->GetPurportedAccountID().asBase58(
                                    api_.Crypto());
                    }
                } break;
                case proto::PAYMENTEVENTTYPE_COMPLETE: {
                    text = "Transfer #" + number + " cleared.";
                } break;
                default: {
                    LogError()(OT_PRETTY_CLASS())("Invalid event state (")(
                        event.type())(")")
                        .Flush();
                }
            }
        } break;
        case otx::client::StorageBox::INTERNALTRANSFER: {
            const auto in =
                parent_.AccountID() ==
                transfer_->GetDestinationAcctID().asBase58(api_.Crypto());

            switch (event.type()) {
                case proto::PAYMENTEVENTTYPE_ACKNOWLEDGE: {
                    if (in) {
                        text = "Received internal transfer #" + number +
                               " from account " +
                               transfer_->GetPurportedAccountID().asBase58(
                                   api_.Crypto());
                    } else {
                        text = "Sent internal transfer #" + number +
                               " to account " +
                               transfer_->GetDestinationAcctID().asBase58(
                                   api_.Crypto());
                    }
                } break;
                case proto::PAYMENTEVENTTYPE_COMPLETE: {
                    text = "Transfer #" + number + " cleared.";
                } break;
                default: {
                    LogError()(OT_PRETTY_CLASS())("Invalid event state (")(
                        event.type())(")")
                        .Flush();
                }
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
        case otx::client::StorageBox::INCOMINGCHEQUE:
        case otx::client::StorageBox::OUTGOINGCHEQUE:
        case otx::client::StorageBox::DRAFT:
        case otx::client::StorageBox::UNKNOWN:
        case otx::client::StorageBox::RESERVED_1:
        case otx::client::StorageBox::PENDING_SEND:
        default: {
            LogError()(OT_PRETTY_CLASS())("Invalid item type (")(
                static_cast<std::uint8_t>(type_))(")")
                .Flush();
        }
    }

    auto output{false};
    lock.lock();

    if (text_ != text) {
        text_ = text;
        output = true;
    }

    lock.unlock();

    return output;
}

auto TransferBalanceItem::UUID() const noexcept -> UnallocatedCString
{
    if (transfer_) {

        return api::session::Workflow::UUID(
                   api_,
                   transfer_->GetPurportedNotaryID(),
                   transfer_->GetTransactionNum())
            .asBase58(api_.Crypto());
    }

    return {};
}
}  // namespace opentxs::ui::implementation
