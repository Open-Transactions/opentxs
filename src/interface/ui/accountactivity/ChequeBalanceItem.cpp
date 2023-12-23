// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/ui/accountactivity/ChequeBalanceItem.hpp"  // IWYU pragma: associated

#include <PaymentEvent.pb.h>
#include <PaymentWorkflow.pb.h>
#include <PaymentWorkflowEnums.pb.h>
#include <cstdint>
#include <memory>

#include "interface/ui/accountactivity/BalanceItem.hpp"
#include "interface/ui/base/Widget.hpp"
#include "internal/core/String.hpp"
#include "internal/otx/common/Cheque.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Workflow.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Notary.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::ui::implementation
{
ChequeBalanceItem::ChequeBalanceItem(
    const AccountActivityInternalInterface& parent,
    const api::session::Client& api,
    const AccountActivityRowID& rowID,
    const AccountActivitySortKey& sortKey,
    CustomData& custom,
    const identifier::Nym& nymID,
    const identifier::Account& accountID) noexcept
    : BalanceItem(parent, api, rowID, sortKey, custom, nymID, accountID)
    , cheque_(nullptr)
{
    startup(
        extract_custom<proto::PaymentWorkflow>(custom, 0),
        extract_custom<proto::PaymentEvent>(custom, 1));
}

auto ChequeBalanceItem::effective_amount() const noexcept -> opentxs::Amount
{
    const auto lock = sLock{shared_lock_};
    auto amount = opentxs::Amount{0};
    auto sign = opentxs::Amount{0};

    if (cheque_) { amount = cheque_->GetAmount(); }

    switch (type_) {
        case otx::client::StorageBox::OUTGOINGCHEQUE: {
            sign = -1;
        } break;
        case otx::client::StorageBox::INCOMINGCHEQUE: {
            sign = 1;
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
        }
    }

    return amount * sign;
}

auto ChequeBalanceItem::Memo() const noexcept -> UnallocatedCString
{
    const auto lock = sLock{shared_lock_};

    if (cheque_) { return cheque_->GetMemo().Get(); }

    return {};
}

auto ChequeBalanceItem::reindex(
    const implementation::AccountActivitySortKey& key,
    implementation::CustomData& custom) noexcept -> bool
{
    auto output = BalanceItem::reindex(key, custom);
    output |= startup(
        extract_custom<proto::PaymentWorkflow>(custom, 0),
        extract_custom<proto::PaymentEvent>(custom, 1));

    return output;
}

auto ChequeBalanceItem::startup(
    const proto::PaymentWorkflow workflow,
    const proto::PaymentEvent event) noexcept -> bool
{
    auto lock = eLock{shared_lock_};

    if (false == bool(cheque_)) {
        cheque_ =
            api::session::Workflow::InstantiateCheque(api_, workflow).second;
    }

    assert_false(nullptr == cheque_);

    lock.unlock();
    auto name = UnallocatedCString{};
    auto text = UnallocatedCString{};
    auto number = std::to_string(cheque_->GetTransactionNum());
    auto otherNymID = identifier::Nym{};

    switch (type_) {
        case otx::client::StorageBox::INCOMINGCHEQUE: {
            otherNymID.Assign(cheque_->GetSenderNymID());

            if (otherNymID.empty()) { otherNymID = nym_id_; }

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
                case proto::PAYMENTEVENTTYPE_ABORT:
                case proto::PAYMENTEVENTTYPE_ACKNOWLEDGE:
                case proto::PAYMENTEVENTTYPE_EXPIRE:
                case proto::PAYMENTEVENTTYPE_REJECT:
                default: {
                    LogError()()("Invalid event state (")(event.type())(")")
                        .Flush();
                }
            }
        } break;
        case otx::client::StorageBox::OUTGOINGCHEQUE: {
            otherNymID.Assign(cheque_->GetRecipientNymID());

            switch (event.type()) {
                case proto::PAYMENTEVENTTYPE_CREATE: {
                    text = "Wrote cheque #" + number;

                    if (false == otherNymID.empty()) {
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
                case proto::PAYMENTEVENTTYPE_ABORT:
                case proto::PAYMENTEVENTTYPE_ACKNOWLEDGE:
                case proto::PAYMENTEVENTTYPE_EXPIRE:
                case proto::PAYMENTEVENTTYPE_REJECT:
                default: {
                    LogError()()("Invalid event state (")(event.type())(")")
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
        case otx::client::StorageBox::OUTGOINGTRANSFER:
        case otx::client::StorageBox::INCOMINGTRANSFER:
        case otx::client::StorageBox::INTERNALTRANSFER:
        case otx::client::StorageBox::DRAFT:
        case otx::client::StorageBox::UNKNOWN:
        case otx::client::StorageBox::RESERVED_1:
        case otx::client::StorageBox::PENDING_SEND:
        default: {
            LogError()()("Invalid item type (")(
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

auto ChequeBalanceItem::UUID() const noexcept -> UnallocatedCString
{
    if (cheque_) {

        return api::session::Workflow::UUID(
                   api_, cheque_->GetNotaryID(), cheque_->GetTransactionNum())
            .asBase58(api_.Crypto());
    }

    return {};
}
}  // namespace opentxs::ui::implementation
