// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/ui/accountactivity/BalanceItem.hpp"  // IWYU pragma: associated

#include <PaymentWorkflow.pb.h>
#include <chrono>
#include <memory>

#include "interface/ui/accountactivity/ChequeBalanceItem.hpp"
#include "interface/ui/accountactivity/TransferBalanceItem.hpp"
#include "internal/api/session/Types.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/display/Definition.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/otx/client/PaymentWorkflowType.hpp"  // IWYU pragma: keep
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto BalanceItem(
    const ui::implementation::AccountActivityInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::AccountActivityRowID& rowID,
    const ui::implementation::AccountActivitySortKey& sortKey,
    ui::implementation::CustomData& custom,
    const identifier::Nym& nymID,
    const identifier::Account& accountID) noexcept
    -> std::shared_ptr<ui::implementation::AccountActivityRowInternal>
{
    if (2_uz < custom.size()) {

        return BalanceItemBlockchain(
            parent, api, rowID, sortKey, custom, nymID, accountID);
    } else {

        return BalanceItemCustodial(
            parent, api, rowID, sortKey, custom, nymID, accountID);
    }
}

auto BalanceItemCustodial(
    const ui::implementation::AccountActivityInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::AccountActivityRowID& rowID,
    const ui::implementation::AccountActivitySortKey& sortKey,
    ui::implementation::CustomData& custom,
    const identifier::Nym& nymID,
    const identifier::Account& accountID) noexcept
    -> std::shared_ptr<ui::implementation::AccountActivityRowInternal>
{
    const auto type =
        ui::implementation::BalanceItem::recover_workflow(custom).type();

    switch (translate(type)) {
        case otx::client::PaymentWorkflowType::OutgoingCheque:
        case otx::client::PaymentWorkflowType::IncomingCheque:
        case otx::client::PaymentWorkflowType::OutgoingInvoice:
        case otx::client::PaymentWorkflowType::IncomingInvoice: {
            return std::make_shared<ui::implementation::ChequeBalanceItem>(
                parent, api, rowID, sortKey, custom, nymID, accountID);
        }
        case otx::client::PaymentWorkflowType::OutgoingTransfer:
        case otx::client::PaymentWorkflowType::IncomingTransfer:
        case otx::client::PaymentWorkflowType::InternalTransfer: {
            return std::make_shared<ui::implementation::TransferBalanceItem>(
                parent, api, rowID, sortKey, custom, nymID, accountID);
        }
        case otx::client::PaymentWorkflowType::Error:
        case otx::client::PaymentWorkflowType::OutgoingCash:
        case otx::client::PaymentWorkflowType::IncomingCash:
        default: {
            LogAbort()()("Unhandled workflow type (")(type)(")").Abort();
        }
    }
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
BalanceItem::BalanceItem(
    const AccountActivityInternalInterface& parent,
    const api::session::Client& api,
    const AccountActivityRowID& rowID,
    const AccountActivitySortKey& sortKey,
    CustomData& custom,
    const identifier::Nym& nymID,
    const identifier::Account& accountID,
    const UnallocatedCString& text) noexcept
    : BalanceItemRow(parent, api, rowID, true)
    , api_(api)
    , nym_id_(nymID)
    , workflow_(recover_workflow(custom).id())
    , type_(extract_type(recover_workflow(custom)))
    , text_(text)
    , time_(sortKey)
    , account_id_(accountID)
    , contacts_(extract_contacts(api, recover_workflow(custom)))
{
}

auto BalanceItem::DisplayAmount() const noexcept -> UnallocatedCString
{
    const auto lock = sLock{shared_lock_};
    const auto& amount = effective_amount();
    const auto& definition =
        display::GetDefinition(parent_.Contract().UnitOfAccount());
    UnallocatedCString output = definition.Format(amount);

    if (0 < output.size()) { return output; }

    amount.Serialize(writer(output));
    return output;
}

auto BalanceItem::extract_contacts(
    const api::session::Client& api,
    const proto::PaymentWorkflow& workflow) noexcept
    -> UnallocatedVector<UnallocatedCString>
{
    UnallocatedVector<UnallocatedCString> output{};

    for (const auto& party : workflow.party()) {
        const auto contactID =
            api.Contacts().NymToContact(api.Factory().NymIDFromBase58(party));
        output.emplace_back(contactID.asBase58(api.Crypto()));
    }

    return output;
}

auto BalanceItem::extract_type(const proto::PaymentWorkflow& workflow) noexcept
    -> otx::client::StorageBox
{
    switch (translate(workflow.type())) {
        case otx::client::PaymentWorkflowType::OutgoingCheque: {

            return otx::client::StorageBox::OUTGOINGCHEQUE;
        }
        case otx::client::PaymentWorkflowType::IncomingCheque: {

            return otx::client::StorageBox::INCOMINGCHEQUE;
        }
        case otx::client::PaymentWorkflowType::OutgoingTransfer: {

            return otx::client::StorageBox::OUTGOINGTRANSFER;
        }
        case otx::client::PaymentWorkflowType::IncomingTransfer: {

            return otx::client::StorageBox::INCOMINGTRANSFER;
        }
        case otx::client::PaymentWorkflowType::InternalTransfer: {

            return otx::client::StorageBox::INTERNALTRANSFER;
        }
        case otx::client::PaymentWorkflowType::Error:
        case otx::client::PaymentWorkflowType::OutgoingInvoice:
        case otx::client::PaymentWorkflowType::IncomingInvoice:
        case otx::client::PaymentWorkflowType::OutgoingCash:
        case otx::client::PaymentWorkflowType::IncomingCash:
        default: {

            return otx::client::StorageBox::UNKNOWN;
        }
    }
}

auto BalanceItem::get_contact_name(const identifier::Nym& nymID) const noexcept
    -> UnallocatedCString
{
    if (nymID.empty()) { return {}; }

    UnallocatedCString output{nymID.asBase58(api_.Crypto())};
    const auto contactID = api_.Contacts().ContactID(nymID);

    if (false == contactID.empty()) {
        output = api_.Contacts().ContactName(contactID);
    }

    return output;
}

auto BalanceItem::recover_workflow(CustomData& custom) noexcept
    -> const proto::PaymentWorkflow&
{
    assert_true(2 <= custom.size());

    const auto& input = custom.at(0);

    assert_false(nullptr == input);

    return *static_cast<const proto::PaymentWorkflow*>(input);
}

auto BalanceItem::reindex(
    const implementation::AccountActivitySortKey& key,
    implementation::CustomData&) noexcept -> bool
{
    const auto lock = eLock{shared_lock_};

    if (key == time_) {

        return false;
    } else {
        time_ = key;

        return true;
    }
}

auto BalanceItem::Text() const noexcept -> UnallocatedCString
{
    const auto lock = sLock{shared_lock_};

    return text_;
}

auto BalanceItem::Timestamp() const noexcept -> Time
{
    const auto lock = sLock{shared_lock_};

    return time_;
}

BalanceItem::~BalanceItem() = default;
}  // namespace opentxs::ui::implementation
