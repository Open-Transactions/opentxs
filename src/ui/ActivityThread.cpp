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

#include "opentxs/stdafx.hpp"

#include "opentxs/api/client/Sync.hpp"
#include "opentxs/api/Activity.hpp"
#include "opentxs/api/ContactManager.hpp"
#include "opentxs/contact/Contact.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/SubscribeSocket.hpp"
#include "opentxs/ui/ActivityThread.hpp"
#include "opentxs/ui/ActivityThreadItem.hpp"
#include "opentxs/Types.hpp"

#include "ActivityThreadItemBlank.hpp"
#include "ActivityThreadParent.hpp"
#include "List.hpp"

#include <map>
#include <memory>
#include <set>
#include <thread>
#include <tuple>
#include <vector>

#include "ActivityThread.hpp"

template class std::
    tuple<opentxs::OTIdentifier, opentxs::StorageBox, opentxs::OTIdentifier>;

#define OT_METHOD "opentxs::ui::implementation::ActivityThread::"

namespace opentxs
{
ui::ActivityThread* Factory::ActivityThread(
    const network::zeromq::Context& zmq,
    const api::client::Sync& sync,
    const api::Activity& activity,
    const api::ContactManager& contact,
    const Identifier& nymID,
    const Identifier& threadID)
{
    return new ui::implementation::ActivityThread(
        zmq, sync, activity, contact, nymID, threadID);
}
}  // namespace opentxs

namespace opentxs::ui::implementation
{
ActivityThread::ActivityThread(
    const network::zeromq::Context& zmq,
    const api::client::Sync& sync,
    const api::Activity& activity,
    const api::ContactManager& contact,
    const Identifier& nymID,
    const Identifier& threadID)
    : ActivityThreadType(
          zmq,
          contact,
          {Identifier::Factory(), {}, Identifier::Factory()},
          nymID,
          new ActivityThreadItemBlank)
    , activity_(activity)
    , sync_(sync)
    , threadID_(Identifier::Factory(threadID))
    , participants_()
    , activity_subscriber_callback_(network::zeromq::ListenCallback::Factory(
          [this](const network::zeromq::Message& message) -> void {
              this->process_thread(message);
          }))
    , activity_subscriber_(
          zmq_.SubscribeSocket(activity_subscriber_callback_.get()))
    , contact_lock_()
    , draft_lock_()
    , draft_()
    , draft_tasks_()
    , contact_(nullptr)
    , contact_thread_(nullptr)
{
    OT_ASSERT(blank_p_)

    init();
    const auto endpoint = activity_.ThreadPublisher(nymID);
    otWarn << OT_METHOD << __FUNCTION__ << ": Connecting to " << endpoint
           << std::endl;
    const auto listening = activity_subscriber_->Start(endpoint);

    OT_ASSERT(listening)

    startup_.reset(new std::thread(&ActivityThread::startup, this));

    OT_ASSERT(startup_)

    contact_thread_.reset(new std::thread(&ActivityThread::init_contact, this));

    OT_ASSERT(contact_thread_)
}

ActivityThreadID ActivityThread::blank_id() const
{
    return {Identifier::Factory(), {}, Identifier::Factory()};
}

bool ActivityThread::check_draft(const ActivityThreadID& id) const
{
    const auto& taskID = std::get<0>(id);
    const auto[status, contactID] = sync_.MessageStatus(taskID);
    [[maybe_unused]] const auto& notUsed = contactID;

    switch (status) {
        case ThreadStatus::RUNNING:
        case ThreadStatus::FINISHED_SUCCESS: {
            otErr << OT_METHOD << __FUNCTION__ << ": Message sent successfully "
                  << std::endl;

            return true;
        } break;
        case ThreadStatus::FINISHED_FAILED: {
            otErr << OT_METHOD << __FUNCTION__ << ": Failed to send message "
                  << std::endl;
        } break;
        case ThreadStatus::ERROR:
        case ThreadStatus::SHUTDOWN:
        default: {
        }
    }

    return false;
}

void ActivityThread::check_drafts() const
{
    eLock lock(draft_lock_);
    otErr << OT_METHOD << __FUNCTION__ << ": Checking " << draft_tasks_.size()
          << " pending sends." << std::endl;
    std::set<ActivityThreadID> deleted{};

    for (const auto& draftID : draft_tasks_) {
        if (check_draft(draftID)) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Removing successfully sent draft." << std::endl;
            Lock lock(lock_);
            delete_item(lock, draftID);
            deleted.emplace(draftID);
        } else {
            otErr << OT_METHOD << __FUNCTION__ << ": Keeping pending send"
                  << std::endl;
        }
    }

    for (const auto& id : deleted) {
        draft_tasks_.erase(id);
    }
}

std::string ActivityThread::comma(const std::set<std::string>& list) const
{
    std::ostringstream stream;

    for (const auto& item : list) {
        stream << item;
        stream << ", ";
    }

    std::string output = stream.str();

    if (0 < output.size()) {
        output.erase(output.size() - 2, 2);
    }

    return output;
}

void ActivityThread::construct_item(
    const ActivityThreadID& id,
    const ActivityThreadSortKey& index,
    void*) const
{
    names_.emplace(id, index);
    const auto& time = std::get<0>(index);
    const auto& box = std::get<1>(id);

    switch (box) {
        case StorageBox::MAILINBOX:
        case StorageBox::MAILOUTBOX: {
            items_[index].emplace(
                id,
                Factory::MailItem(
                    *this,
                    zmq_,
                    contact_manager_,
                    id,
                    nym_id_,
                    activity_,
                    time));
        } break;
        case StorageBox::DRAFT: {
            items_[index].emplace(
                id,
                Factory::MailItem(
                    *this,
                    zmq_,
                    contact_manager_,
                    id,
                    nym_id_,
                    activity_,
                    time,
                    draft_,
                    false,
                    true));
        } break;
        case StorageBox::INCOMINGCHEQUE:
        case StorageBox::OUTGOINGCHEQUE: {
            items_[index].emplace(
                id,
                Factory::PaymentItem(
                    *this,
                    zmq_,
                    contact_manager_,
                    id,
                    nym_id_,
                    activity_,
                    time));
        } break;
        case StorageBox::SENTPEERREQUEST:
        case StorageBox::INCOMINGPEERREQUEST:
        case StorageBox::SENTPEERREPLY:
        case StorageBox::INCOMINGPEERREPLY:
        case StorageBox::FINISHEDPEERREQUEST:
        case StorageBox::FINISHEDPEERREPLY:
        case StorageBox::PROCESSEDPEERREQUEST:
        case StorageBox::PROCESSEDPEERREPLY:
        case StorageBox::INCOMINGBLOCKCHAIN:
        case StorageBox::OUTGOINGBLOCKCHAIN:
        case StorageBox::UNKNOWN:
        default: {
            OT_FAIL
        }
    }
}

std::string ActivityThread::DisplayName() const
{
    Lock lock(lock_);

    std::set<std::string> names{};

    for (const auto& contactID : participants_) {
        auto name = contact_manager_.ContactName(contactID);

        if (name.empty()) {
            names.emplace(contactID->str());
        } else {
            names.emplace(name);
        }
    }

    return comma(names);
}

std::string ActivityThread::GetDraft() const
{
    sLock lock(draft_lock_);

    return draft_;
}

void ActivityThread::init_contact()
{
    if (1 != participants_.size()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Wrong number of participants ("
              << participants_.size() << ")" << std::endl;

        return;
    }

    auto contact = contact_manager_.Contact(*participants_.cbegin());
    Lock lock(contact_lock_);
    contact_ = contact;
    lock.unlock();
    UpdateNotify();
}

void ActivityThread::load_thread(const proto::StorageThread& thread)
{
    for (const auto& id : thread.participant()) {
        participants_.emplace(Identifier::Factory(id));
    }

    otWarn << OT_METHOD << __FUNCTION__ << ": Loading " << thread.item().size()
           << " items." << std::endl;

    for (const auto& item : thread.item()) {
        process_item(item);
    }

    UpdateNotify();
    startup_complete_->On();
}

void ActivityThread::new_thread()
{
    participants_.emplace(threadID_);
    UpdateNotify();
    startup_complete_->On();
}

ActivityThreadOuter::const_iterator ActivityThread::outer_first() const
{
    return items_.begin();
}

ActivityThreadOuter::const_iterator ActivityThread::outer_end() const
{
    return items_.end();
}

std::string ActivityThread::Participants() const
{
    Lock lock(lock_);
    std::set<std::string> ids{};

    for (const auto& id : participants_) {
        ids.emplace(id->str());
    }

    return comma(ids);
}

std::string ActivityThread::PaymentCode(
    const proto::ContactItemType currency) const
{
    Lock lock(contact_lock_);

    if (contact_) {

        return contact_->PaymentCode(currency);
    }

    return {};
}

ActivityThreadID ActivityThread::process_item(
    const proto::StorageThreadItem& item)
{
    const ActivityThreadID id{Identifier::Factory(item.id()),
                              static_cast<StorageBox>(item.box()),
                              Identifier::Factory(item.account())};
    const ActivityThreadSortKey key{std::chrono::seconds(item.time()),
                                    item.index()};
    add_item(id, key);
    UpdateNotify();

    return id;
}

void ActivityThread::process_thread(const network::zeromq::Message& message)
{
    wait_for_startup();
    check_drafts();
    const std::string id(message);
    const auto threadID = Identifier::Factory(id);

    OT_ASSERT(false == threadID->empty())

    if (threadID_ != threadID) {

        return;
    }

    const auto thread = activity_.Thread(nym_id_, threadID_);

    OT_ASSERT(thread)

    std::set<ActivityThreadID> active{};

    for (const auto& item : thread->item()) {
        const auto id = process_item(item);
        active.emplace(id);
    }

    delete_inactive(active);
}

bool ActivityThread::same(
    const ActivityThreadID& lhs,
    const ActivityThreadID& rhs) const
{
    const auto & [ lID, lBox, lAccount ] = lhs;
    const auto & [ rID, rBox, rAccount ] = rhs;
    const bool sameID = (lID->str() == rID->str());
    const bool sameBox = (lBox == rBox);
    const bool sameAccount = (lAccount->str() == rAccount->str());

    return sameID && sameBox && sameAccount;
}

bool ActivityThread::SendDraft() const
{
    eLock draftLock(draft_lock_);

    if (draft_.empty()) {
        otErr << OT_METHOD << __FUNCTION__ << ": No draft message to send."
              << std::endl;

        return false;
    }

    if (1 != participants_.size()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Sending to multiple recipient not yet supported."
              << std::endl;

        return false;
    }

    const auto taskID =
        sync_.MessageContact(nym_id_, *participants_.begin(), draft_);

    // if (taskID.empty())
    if (taskID->empty()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to queue message for sending" << std::endl;

        return false;
    }

    const ActivityThreadID id{taskID, StorageBox::DRAFT, Identifier::Factory()};
    const ActivityThreadSortKey key{std::chrono::system_clock::now(), 0};
    draft_tasks_.insert(id);
    draft_.clear();
    const_cast<ActivityThread&>(*this).add_item(id, key);
    UpdateNotify();

    return true;
}

bool ActivityThread::SetDraft(const std::string& draft) const
{
    if (draft.empty()) {

        return false;
    }

    eLock lock(draft_lock_);
    draft_ = draft;

    return true;
}

void ActivityThread::startup()
{
    const auto thread = activity_.Thread(nym_id_, threadID_);

    if (thread) {
        load_thread(*thread);
    } else {
        new_thread();
    }
}

std::string ActivityThread::ThreadID() const
{
    Lock lock(lock_);

    return threadID_->str();
}

ActivityThread::~ActivityThread()
{
    if (contact_thread_ && contact_thread_->joinable()) {
        contact_thread_->join();
        contact_thread_.reset();
    }
}
}  // namespace opentxs::ui::implementation
