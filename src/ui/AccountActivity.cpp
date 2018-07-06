/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#include "stdafx.hpp"

#include "opentxs/api/client/Wallet.hpp"
#include "opentxs/api/client/Workflow.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/Socket.hpp"
#include "opentxs/network/zeromq/SubscribeSocket.hpp"
#include "opentxs/ui/AccountActivity.hpp"
#include "opentxs/ui/BalanceItem.hpp"

#include "BalanceItemBlank.hpp"
#include "AccountActivityParent.hpp"
#include "List.hpp"

#include <atomic>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <thread>
#include <tuple>
#include <vector>

#include "AccountActivity.hpp"

#define OT_METHOD "opentxs::ui::implementation::AccountActivity::"

namespace opentxs
{
ui::AccountActivity* Factory::AccountActivity(
    const network::zeromq::Context& zmq,
    const network::zeromq::PublishSocket& publisher,
    const api::client::Sync& sync,
    const api::client::Wallet& wallet,
    const api::client::Workflow& workflow,
    const api::ContactManager& contact,
    const api::storage::Storage& storage,
    const Identifier& nymID,
    const Identifier& accountID)
{
    return new ui::implementation::AccountActivity(
        zmq,
        publisher,
        sync,
        wallet,
        workflow,
        contact,
        storage,
        nymID,
        accountID);
}
}  // namespace opentxs

namespace opentxs::ui::implementation
{
const Widget::ListenerDefinitions AccountActivity::listeners_{
    {network::zeromq::Socket::WorkflowAccountUpdateEndpoint,
     new MessageProcessor<AccountActivity>(&AccountActivity::process_workflow)},
    {network::zeromq::Socket::AccountUpdateEndpoint,
     new MessageProcessor<AccountActivity>(&AccountActivity::process_balance)},
};

AccountActivity::AccountActivity(
    const network::zeromq::Context& zmq,
    const network::zeromq::PublishSocket& publisher,
    const api::client::Sync& sync,
    const api::client::Wallet& wallet,
    const api::client::Workflow& workflow,
    const api::ContactManager& contact,
    const api::storage::Storage& storage,
    const Identifier& nymID,
    const Identifier& accountID)
    : AccountActivityList(nymID, zmq, publisher, contact)
    , sync_(sync)
    , wallet_(wallet)
    , workflow_(workflow)
    , storage_(storage)
    , balance_(0)
    , account_id_(accountID)
    , contract_(nullptr)
{
    init();
    setup_listeners(listeners_);
    startup_.reset(new std::thread(&AccountActivity::startup, this));

    OT_ASSERT(startup_)
}

void AccountActivity::construct_row(
    const AccountActivityRowID& id,
    const AccountActivitySortKey& index,
    const CustomData& custom) const
{
    OT_ASSERT(2 == custom.size())

    items_[index].emplace(
        id,
        Factory::BalanceItem(
            *this,
            zmq_,
            publisher_,
            contact_manager_,
            sync_,
            wallet_,
            recover_workflow(custom[0]),
            recover_event(custom[1]),
            nym_id_,
            account_id_));
    names_.emplace(id, index);
}

std::string AccountActivity::DisplayBalance() const
{
    sLock lock(shared_lock_);

    if (contract_) {
        const auto amount = balance_.load();
        std::string output{};
        const auto formatted =
            contract_->FormatAmountLocale(amount, output, ",", ".");

        if (formatted) { return output; }

        return std::to_string(amount);
    }

    return {};
}

AccountActivity::EventRow AccountActivity::extract_event(
    const proto::PaymentEventType eventType,
    const proto::PaymentWorkflow& workflow)
{
    bool success{false};
    bool found{false};
    EventRow output{};
    auto& [time, event_p] = output;

    for (const auto& event : workflow.event()) {
        const auto eventTime =
            std::chrono::system_clock::from_time_t(event.time());

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

    OT_ASSERT(found)

    return output;
}

std::vector<AccountActivity::RowKey> AccountActivity::extract_rows(
    const proto::PaymentWorkflow& workflow)
{
    std::vector<AccountActivity::RowKey> output;

    switch (workflow.type()) {
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE: {
            switch (workflow.state()) {
                case proto::PAYMENTWORKFLOWSTATE_UNSENT:
                case proto::PAYMENTWORKFLOWSTATE_CONVEYED:
                case proto::PAYMENTWORKFLOWSTATE_EXPIRED: {
                    output.emplace_back(
                        proto::PAYMENTEVENTTYPE_CREATE,
                        extract_event(
                            proto::PAYMENTEVENTTYPE_CREATE, workflow));
                } break;
                case proto::PAYMENTWORKFLOWSTATE_CANCELLED: {
                    output.emplace_back(
                        proto::PAYMENTEVENTTYPE_CREATE,
                        extract_event(
                            proto::PAYMENTEVENTTYPE_CREATE, workflow));
                    output.emplace_back(
                        proto::PAYMENTEVENTTYPE_CANCEL,
                        extract_event(
                            proto::PAYMENTEVENTTYPE_CANCEL, workflow));
                } break;
                case proto::PAYMENTWORKFLOWSTATE_ACCEPTED:
                case proto::PAYMENTWORKFLOWSTATE_COMPLETED: {
                    output.emplace_back(
                        proto::PAYMENTEVENTTYPE_CREATE,
                        extract_event(
                            proto::PAYMENTEVENTTYPE_CREATE, workflow));
                    output.emplace_back(
                        proto::PAYMENTEVENTTYPE_ACCEPT,
                        extract_event(
                            proto::PAYMENTEVENTTYPE_ACCEPT, workflow));
                } break;
                case proto::PAYMENTWORKFLOWSTATE_ERROR:
                default: {
                    otErr << OT_METHOD << __FUNCTION__
                          << ": Invalid workflow state" << std::endl;

                    OT_FAIL
                }
            }
        } break;
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE: {
            switch (workflow.state()) {
                case proto::PAYMENTWORKFLOWSTATE_CONVEYED:
                case proto::PAYMENTWORKFLOWSTATE_EXPIRED:
                case proto::PAYMENTWORKFLOWSTATE_COMPLETED: {
                    output.emplace_back(
                        proto::PAYMENTEVENTTYPE_CONVEY,
                        extract_event(
                            proto::PAYMENTEVENTTYPE_CONVEY, workflow));
                } break;
                case proto::PAYMENTWORKFLOWSTATE_ERROR:
                case proto::PAYMENTWORKFLOWSTATE_UNSENT:
                case proto::PAYMENTWORKFLOWSTATE_CANCELLED:
                case proto::PAYMENTWORKFLOWSTATE_ACCEPTED:
                default: {
                    otErr << OT_METHOD << __FUNCTION__
                          << ": Invalid workflow state ("
                          << std::to_string(workflow.state()) << ")"
                          << std::endl;

                    OT_FAIL
                }
            }
        } break;
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGINVOICE:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGINVOICE:
        case proto::PAYMENTWORKFLOWTYPE_ERROR:
        default: {
            otErr << OT_METHOD << __FUNCTION__ << ": Unsupported workflow type"
                  << std::endl;
        }
    }

    return output;
}

void AccountActivity::process_balance(
    const opentxs::network::zeromq::Message& message)
{
    wait_for_startup();

    OT_ASSERT(2 == message.Body().size())

    const auto accountID = Identifier::Factory(message.Body().at(0));

    if (account_id_ != accountID) { return; }

    const auto& balance = message.Body().at(1);

    OT_ASSERT(balance.size() == sizeof(Amount))

    balance_.store(*static_cast<const Amount*>(balance.data()));
    UpdateNotify();
}

void AccountActivity::process_workflow(
    const Identifier& workflowID,
    std::set<AccountActivityRowID>& active)
{
    const auto workflow = workflow_.LoadWorkflow(nym_id_, workflowID);

    OT_ASSERT(workflow)

    const auto rows = extract_rows(*workflow);

    for (const auto& [type, row] : rows) {
        const auto& [time, event_p] = row;
        AccountActivityRowID key{Identifier::Factory(workflowID), type};
        add_item(key, time, {workflow.get(), event_p});
        active.emplace(std::move(key));
    }
}

void AccountActivity::process_workflow(const network::zeromq::Message& message)
{
    wait_for_startup();

    OT_ASSERT(1 == message.Body().size());

    const std::string id(*message.Body().begin());
    const auto accountID = Identifier::Factory(id);

    OT_ASSERT(false == accountID->empty())

    if (account_id_ == accountID) { startup(); }
}

const proto::PaymentEvent& AccountActivity::recover_event(const void* input)
{
    OT_ASSERT(nullptr != input)

    return *static_cast<const proto::PaymentEvent*>(input);
}

const proto::PaymentWorkflow& AccountActivity::recover_workflow(
    const void* input)
{
    OT_ASSERT(nullptr != input)

    return *static_cast<const proto::PaymentWorkflow*>(input);
}

void AccountActivity::startup()
{
    auto account = wallet_.Account(account_id_);

    if (account) {
        balance_.store(account.get().GetBalance());
        UpdateNotify();
        eLock lock(shared_lock_);
        contract_ =
            wallet_.UnitDefinition(storage_.AccountContract(account_id_));
    }

    account.Release();
    const auto workflows = workflow_.WorkflowsByAccount(nym_id_, account_id_);
    otWarn << OT_METHOD << __FUNCTION__ << ": Loading " << workflows.size()
           << " workflows." << std::endl;
    std::set<AccountActivityRowID> active{};

    for (const auto& id : workflows) { process_workflow(id, active); }

    delete_inactive(active);
    UpdateNotify();
    startup_complete_->On();
    otWarn << OT_METHOD << __FUNCTION__ << ": Loaded " << names_.size()
           << " events." << std::endl;
}

void AccountActivity::update(
    AccountActivityRowInterface& row,
    const CustomData& custom) const
{
    OT_ASSERT(2 == custom.size())

    const auto& workflow = recover_workflow(custom[0]);
    const auto& event = recover_event(custom[1]);
    row.Update(workflow, event);
}
}  // namespace opentxs::ui::implementation
