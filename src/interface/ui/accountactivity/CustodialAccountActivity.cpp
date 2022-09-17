// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "interface/ui/accountactivity/CustodialAccountActivity.hpp"  // IWYU pragma: associated

#include <PaymentEvent.pb.h>
#include <PaymentWorkflow.pb.h>
#include <PaymentWorkflowEnums.pb.h>
#include <atomic>
#include <chrono>
#include <compare>
#include <future>
#include <memory>

#include "Proto.hpp"
#include "interface/ui/base/Widget.hpp"
#include "internal/api/session/Types.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/core/Factory.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/Shared.hpp"
#include "internal/util/Time.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"  // IWYU pragma: keep
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/api/session/Workflow.hpp"
#include "opentxs/core/AccountType.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/contract/Unit.hpp"
#include "opentxs/core/display/Definition.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/otx/client/PaymentWorkflowState.hpp"
#include "opentxs/otx/client/PaymentWorkflowType.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto CustodialAccountActivityModel(
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const identifier::Generic& accountID,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::AccountActivity>
{
    using ReturnType = ui::implementation::CustodialAccountActivity;

    return std::make_unique<ReturnType>(api, nymID, accountID, cb);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
CustodialAccountActivity::CustodialAccountActivity(
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const identifier::Generic& accountID,
    const SimpleCallback& cb) noexcept
    : AccountActivity(api, nymID, accountID, AccountType::Custodial, cb)
    , alias_()
{
    init({
        UnallocatedCString{api.Endpoints().AccountUpdate()},
        UnallocatedCString{api.Endpoints().ContactUpdate()},
        UnallocatedCString{api.Endpoints().ServerUpdate()},
        UnallocatedCString{api.Endpoints().UnitUpdate()},
        UnallocatedCString{api.Endpoints().WorkflowAccountUpdate()},
    });

    // If an Account exists, then the unit definition and notary contracts must
    // exist already.
    OT_ASSERT(0 < Contract().Version());
    OT_ASSERT(0 < Notary().Version());
}

auto CustodialAccountActivity::ContractID() const noexcept -> UnallocatedCString
{
    auto lock = sLock{shared_lock_};

    return contract_->ID().asBase58(api_.Crypto());
}

auto CustodialAccountActivity::display_balance(
    opentxs::Amount amount) const noexcept -> UnallocatedCString
{
    auto lock = sLock{shared_lock_};
    const auto& definition = display::GetDefinition(Contract().UnitOfAccount());
    UnallocatedCString output = definition.Format(amount);

    if (0 < output.size()) { return output; }

    amount.Serialize(writer(output));
    return output;
}

auto CustodialAccountActivity::DisplayUnit() const noexcept
    -> UnallocatedCString
{
    auto lock = sLock{shared_lock_};
    const auto& definition = display::GetDefinition(Unit());
    return UnallocatedCString{definition.ShortName()};
}

auto CustodialAccountActivity::extract_event(
    const proto::PaymentEventType eventType,
    const proto::PaymentWorkflow& workflow) noexcept -> EventRow
{
    bool success{false};
    bool found{false};
    EventRow output{};
    auto& [time, event_p] = output;

    for (const auto& event : workflow.event()) {
        const auto eventTime = convert_stime(event.time());

        if (eventType != event.type()) { continue; }

        if (eventTime > time) {
            if (success) {
                if (event.success()) {
                    time = eventTime;
                    event_p = &event;
                    found = true;
                }
            } else {
                time = eventTime;
                event_p = &event;
                success = event.success();
                found = true;
            }
        } else {
            if (false == success) {
                if (event.success()) {
                    // This is a weird case. It probably shouldn't happen
                    time = eventTime;
                    event_p = &event;
                    success = true;
                    found = true;
                }
            }
        }
    }

    if (false == found) {
        LogError()(OT_PRETTY_STATIC(CustodialAccountActivity))("Workflow ")(
            workflow.id())(", type ")(workflow.type())(", state ")(
            workflow.state())(" does not contain an event of type ")(eventType)
            .Flush();

        OT_FAIL;
    }

    return output;
}

auto CustodialAccountActivity::extract_rows(
    const proto::PaymentWorkflow& workflow) noexcept
    -> UnallocatedVector<RowKey>
{
    auto output = UnallocatedVector<RowKey>{};

    switch (translate(workflow.type())) {
        case otx::client::PaymentWorkflowType::OutgoingCheque: {
            switch (translate(workflow.state())) {
                case otx::client::PaymentWorkflowState::Unsent:
                case otx::client::PaymentWorkflowState::Conveyed:
                case otx::client::PaymentWorkflowState::Expired: {
                    output.emplace_back(
                        proto::PAYMENTEVENTTYPE_CREATE,
                        extract_event(
                            proto::PAYMENTEVENTTYPE_CREATE, workflow));
                } break;
                case otx::client::PaymentWorkflowState::Cancelled: {
                    output.emplace_back(
                        proto::PAYMENTEVENTTYPE_CREATE,
                        extract_event(
                            proto::PAYMENTEVENTTYPE_CREATE, workflow));
                    output.emplace_back(
                        proto::PAYMENTEVENTTYPE_CANCEL,
                        extract_event(
                            proto::PAYMENTEVENTTYPE_CANCEL, workflow));
                } break;
                case otx::client::PaymentWorkflowState::Accepted:
                case otx::client::PaymentWorkflowState::Completed: {
                    output.emplace_back(
                        proto::PAYMENTEVENTTYPE_CREATE,
                        extract_event(
                            proto::PAYMENTEVENTTYPE_CREATE, workflow));
                    output.emplace_back(
                        proto::PAYMENTEVENTTYPE_ACCEPT,
                        extract_event(
                            proto::PAYMENTEVENTTYPE_ACCEPT, workflow));
                } break;
                case otx::client::PaymentWorkflowState::Error:
                case otx::client::PaymentWorkflowState::Initiated:
                case otx::client::PaymentWorkflowState::Aborted:
                case otx::client::PaymentWorkflowState::Acknowledged:
                case otx::client::PaymentWorkflowState::Rejected:
                default: {
                    LogError()(OT_PRETTY_STATIC(CustodialAccountActivity))(
                        "Invalid workflow state (")(workflow.state())(")")
                        .Flush();
                }
            }
        } break;
        case otx::client::PaymentWorkflowType::IncomingCheque: {
            switch (translate(workflow.state())) {
                case otx::client::PaymentWorkflowState::Conveyed:
                case otx::client::PaymentWorkflowState::Expired:
                case otx::client::PaymentWorkflowState::Completed: {
                    output.emplace_back(
                        proto::PAYMENTEVENTTYPE_CONVEY,
                        extract_event(
                            proto::PAYMENTEVENTTYPE_CONVEY, workflow));
                } break;
                case otx::client::PaymentWorkflowState::Error:
                case otx::client::PaymentWorkflowState::Unsent:
                case otx::client::PaymentWorkflowState::Cancelled:
                case otx::client::PaymentWorkflowState::Accepted:
                case otx::client::PaymentWorkflowState::Initiated:
                case otx::client::PaymentWorkflowState::Aborted:
                case otx::client::PaymentWorkflowState::Acknowledged:
                case otx::client::PaymentWorkflowState::Rejected:
                default: {
                    LogError()(OT_PRETTY_STATIC(CustodialAccountActivity))(
                        "Invalid workflow state (")(workflow.state())(")")
                        .Flush();
                }
            }
        } break;
        case otx::client::PaymentWorkflowType::OutgoingTransfer: {
            switch (translate(workflow.state())) {
                case otx::client::PaymentWorkflowState::Acknowledged:
                case otx::client::PaymentWorkflowState::Accepted: {
                    output.emplace_back(
                        proto::PAYMENTEVENTTYPE_ACKNOWLEDGE,
                        extract_event(
                            proto::PAYMENTEVENTTYPE_ACKNOWLEDGE, workflow));
                } break;
                case otx::client::PaymentWorkflowState::Completed: {
                    output.emplace_back(
                        proto::PAYMENTEVENTTYPE_ACKNOWLEDGE,
                        extract_event(
                            proto::PAYMENTEVENTTYPE_ACKNOWLEDGE, workflow));
                    output.emplace_back(
                        proto::PAYMENTEVENTTYPE_COMPLETE,
                        extract_event(
                            proto::PAYMENTEVENTTYPE_COMPLETE, workflow));
                } break;
                case otx::client::PaymentWorkflowState::Initiated:
                case otx::client::PaymentWorkflowState::Aborted: {
                } break;
                case otx::client::PaymentWorkflowState::Error:
                case otx::client::PaymentWorkflowState::Unsent:
                case otx::client::PaymentWorkflowState::Conveyed:
                case otx::client::PaymentWorkflowState::Cancelled:
                case otx::client::PaymentWorkflowState::Expired:
                case otx::client::PaymentWorkflowState::Rejected:
                default: {
                    LogError()(OT_PRETTY_STATIC(CustodialAccountActivity))(
                        "Invalid workflow state (")(workflow.state())(")")
                        .Flush();
                }
            }
        } break;
        case otx::client::PaymentWorkflowType::IncomingTransfer: {
            switch (translate(workflow.state())) {
                case otx::client::PaymentWorkflowState::Conveyed: {
                    output.emplace_back(
                        proto::PAYMENTEVENTTYPE_CONVEY,
                        extract_event(
                            proto::PAYMENTEVENTTYPE_CONVEY, workflow));
                } break;
                case otx::client::PaymentWorkflowState::Completed: {
                    output.emplace_back(
                        proto::PAYMENTEVENTTYPE_CONVEY,
                        extract_event(
                            proto::PAYMENTEVENTTYPE_CONVEY, workflow));
                    output.emplace_back(
                        proto::PAYMENTEVENTTYPE_ACCEPT,
                        extract_event(
                            proto::PAYMENTEVENTTYPE_ACCEPT, workflow));
                } break;
                case otx::client::PaymentWorkflowState::Error:
                case otx::client::PaymentWorkflowState::Unsent:
                case otx::client::PaymentWorkflowState::Cancelled:
                case otx::client::PaymentWorkflowState::Accepted:
                case otx::client::PaymentWorkflowState::Expired:
                case otx::client::PaymentWorkflowState::Initiated:
                case otx::client::PaymentWorkflowState::Aborted:
                case otx::client::PaymentWorkflowState::Acknowledged:
                case otx::client::PaymentWorkflowState::Rejected:
                default: {
                    LogError()(OT_PRETTY_STATIC(CustodialAccountActivity))(
                        "Invalid workflow state (")(workflow.state())(")")
                        .Flush();
                }
            }
        } break;
        case otx::client::PaymentWorkflowType::InternalTransfer: {
            switch (translate(workflow.state())) {
                case otx::client::PaymentWorkflowState::Acknowledged:
                case otx::client::PaymentWorkflowState::Conveyed:
                case otx::client::PaymentWorkflowState::Accepted: {
                    output.emplace_back(
                        proto::PAYMENTEVENTTYPE_ACKNOWLEDGE,
                        extract_event(
                            proto::PAYMENTEVENTTYPE_ACKNOWLEDGE, workflow));
                } break;
                case otx::client::PaymentWorkflowState::Completed: {
                    output.emplace_back(
                        proto::PAYMENTEVENTTYPE_ACKNOWLEDGE,
                        extract_event(
                            proto::PAYMENTEVENTTYPE_ACKNOWLEDGE, workflow));
                    output.emplace_back(
                        proto::PAYMENTEVENTTYPE_COMPLETE,
                        extract_event(
                            proto::PAYMENTEVENTTYPE_COMPLETE, workflow));
                } break;
                case otx::client::PaymentWorkflowState::Initiated:
                case otx::client::PaymentWorkflowState::Aborted: {
                } break;
                case otx::client::PaymentWorkflowState::Error:
                case otx::client::PaymentWorkflowState::Unsent:
                case otx::client::PaymentWorkflowState::Cancelled:
                case otx::client::PaymentWorkflowState::Expired:
                case otx::client::PaymentWorkflowState::Rejected:
                default: {
                    LogError()(OT_PRETTY_STATIC(CustodialAccountActivity))(
                        "Invalid workflow state (")(workflow.state())(")")
                        .Flush();
                }
            }
        } break;
        case otx::client::PaymentWorkflowType::Error:
        case otx::client::PaymentWorkflowType::OutgoingInvoice:
        case otx::client::PaymentWorkflowType::IncomingInvoice:
        case otx::client::PaymentWorkflowType::OutgoingCash:
        case otx::client::PaymentWorkflowType::IncomingCash:
        default: {
            LogError()(OT_PRETTY_STATIC(CustodialAccountActivity))(
                "Unsupported workflow type (")(workflow.type())(")")
                .Flush();
        }
    }

    return output;
}

auto CustodialAccountActivity::Name() const noexcept -> UnallocatedCString
{
    const auto& api = api_;

    return account_name_custodial(
        api,
        api.Storage().AccountServer(account_id_),
        api.Storage().AccountContract(account_id_),
        [this] {
            auto lock = sLock{shared_lock_};

            return alias_;
        }());
}

auto CustodialAccountActivity::NotaryID() const noexcept -> UnallocatedCString
{
    auto lock = sLock{shared_lock_};

    return notary_->ID().asBase58(api_.Crypto());
}

auto CustodialAccountActivity::NotaryName() const noexcept -> UnallocatedCString
{
    auto lock = sLock{shared_lock_};

    return notary_->EffectiveName();
}

auto CustodialAccountActivity::pipeline(const Message& in) noexcept -> void
{
    if (false == running_.load()) { return; }

    const auto body = in.Body();

    if (1 > body.size()) {
        LogError()(OT_PRETTY_CLASS())("Invalid message").Flush();

        OT_FAIL;
    }

    const auto work = [&] {
        try {

            return body.at(0).as<Work>();
        } catch (...) {

            OT_FAIL;
        }
    }();

    switch (work) {
        case Work::notary: {
            process_notary(in);
        } break;
        case Work::unit: {
            process_unit(in);
        } break;
        case Work::contact: {
            process_contact(in);
        } break;
        case Work::account: {
            process_balance(in);
        } break;
        case Work::workflow: {
            process_workflow(in);
        } break;
        case Work::init: {
            startup();
            finish_startup();
        } break;
        case Work::statemachine: {
            do_work();
        } break;
        case Work::shutdown: {
            if (auto previous = running_.exchange(false); previous) {
                shutdown(shutdown_promise_);
            }
        } break;
        default: {
            LogError()(OT_PRETTY_CLASS())("Unhandled type").Flush();

            OT_FAIL;
        }
    }
}

auto CustodialAccountActivity::process_balance(const Message& message) noexcept
    -> void
{
    wait_for_startup();
    const auto body = message.Body();

    OT_ASSERT(2 < body.size());

    const auto accountID =
        api_.Factory().IdentifierFromHash(body.at(1).Bytes());

    if (account_id_ != accountID) { return; }

    const auto balance = factory::Amount(body.at(2));
    const auto oldBalance = [&] {
        eLock lock(shared_lock_);

        const auto oldbalance = balance_;
        balance_ = balance;
        return oldbalance;
    }();
    const auto balanceChanged = (oldBalance != balance);
    const auto alias = [&] {
        auto account = api_.Wallet().Internal().Account(account_id_);

        OT_ASSERT(account);

        return account.get().Alias();
    }();
    const auto aliasChanged = [&] {
        eLock lock(shared_lock_);

        if (alias != alias_) {
            alias_ = alias;

            return true;
        }

        return false;
    }();

    if (balanceChanged) { notify_balance(balance); }

    if (aliasChanged) { UpdateNotify(); }
}

auto CustodialAccountActivity::process_contact(const Message& message) noexcept
    -> void
{
    wait_for_startup();
    // Contact names may have changed, therefore all row texts must be
    // recalculated
    startup();
}

auto CustodialAccountActivity::process_notary(const Message& message) noexcept
    -> void
{
    wait_for_startup();
    const auto oldName = NotaryName();
    const auto newName = [&] {
        {
            eLock lock{shared_lock_};
            notary_ =
                api_.Wallet().Server(api_.Storage().AccountServer(account_id_));
        }

        return NotaryName();
    }();

    if (oldName != newName) {
        // TODO Qt widgets need to know that notary name property has changed
        UpdateNotify();
    }
}

auto CustodialAccountActivity::process_workflow(
    const identifier::Generic& workflowID,
    UnallocatedSet<AccountActivityRowID>& active) noexcept -> void
{
    const auto workflow = [&] {
        auto out = proto::PaymentWorkflow{};
        api_.Workflow().LoadWorkflow(primary_id_, workflowID, out);

        return out;
    }();
    const auto rows = extract_rows(workflow);

    for (const auto& [type, row] : rows) {
        const auto& [time, event_p] = row;
        auto key = AccountActivityRowID{workflowID, type};
        auto custom = CustomData{
            new proto::PaymentWorkflow(workflow),
            new proto::PaymentEvent(*event_p)};
        add_item(key, time, custom);
        active.emplace(std::move(key));
    }
}

auto CustodialAccountActivity::process_workflow(const Message& message) noexcept
    -> void
{
    wait_for_startup();
    const auto body = message.Body();

    OT_ASSERT(1 < body.size());

    const auto accountID =
        api_.Factory().IdentifierFromHash(body.at(1).Bytes());

    OT_ASSERT(false == accountID.empty());

    if (account_id_ == accountID) { startup(); }
}

auto CustodialAccountActivity::process_unit(const Message& message) noexcept
    -> void
{
    wait_for_startup();
    // TODO currently it doesn't matter if the unit definition alias changes
    // since we don't use it
    eLock lock{shared_lock_};
    contract_ = api_.Wallet().UnitDefinition(
        api_.Storage().AccountContract(account_id_));
}

auto CustodialAccountActivity::startup() noexcept -> void
{
    const auto alias = [&] {
        auto account = api_.Wallet().Internal().Account(account_id_);

        OT_ASSERT(account);

        return account.get().Alias();
    }();
    const auto aliasChanged = [&] {
        eLock lock(shared_lock_);

        if (alias != alias_) {
            alias_ = alias;

            return true;
        }

        return false;
    }();

    const auto workflows =
        api_.Workflow().WorkflowsByAccount(primary_id_, account_id_);
    auto active = UnallocatedSet<AccountActivityRowID>{};

    for (const auto& id : workflows) { process_workflow(id, active); }

    delete_inactive(active);

    if (aliasChanged) {
        // TODO Qt widgets need to know the alias property has changed
        UpdateNotify();
    }
}

auto CustodialAccountActivity::Unit() const noexcept -> UnitType
{
    auto lock = sLock{shared_lock_};

    return contract_->UnitOfAccount();
}

CustodialAccountActivity::~CustodialAccountActivity()
{
    wait_for_startup();
    signal_shutdown().get();
}
}  // namespace opentxs::ui::implementation
