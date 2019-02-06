// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Activity.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/OTX.hpp"
#include "opentxs/contact/Contact.hpp"
#include "opentxs/contact/ContactData.hpp"
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
#include "opentxs/network/zeromq/SubscribeSocket.hpp"
#include "opentxs/ui/ActivityThread.hpp"
#include "opentxs/ui/ActivityThreadItem.hpp"
#include "opentxs/Types.hpp"

#include "ActivityThreadItemBlank.hpp"
#include "InternalUI.hpp"
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
    const api::client::Manager& api,
    const network::zeromq::PublishSocket& publisher,
    const Identifier& nymID,
    const Identifier& threadID)
{
    return new ui::implementation::ActivityThread(
        api, publisher, nymID, threadID);
}
}  // namespace opentxs

namespace opentxs::ui::implementation
{
ActivityThread::ActivityThread(
    const api::client::Manager& api,
    const network::zeromq::PublishSocket& publisher,
    const Identifier& nymID,
    const Identifier& threadID)
    : ActivityThreadList(api, publisher, nymID)
    , listeners_{{api_.Activity().ThreadPublisher(nymID),
        new MessageProcessor<ActivityThread>(&ActivityThread::process_thread)},}
    , threadID_(Identifier::Factory(threadID))
    , participants_()
    , contact_lock_()
    , draft_lock_()
    , draft_()
    , draft_tasks_()
    , contact_(nullptr)
    , contact_thread_(nullptr)
{
    init();
    setup_listeners(listeners_);
    startup_.reset(new std::thread(&ActivityThread::startup, this));

    OT_ASSERT(startup_)

    contact_thread_.reset(new std::thread(&ActivityThread::init_contact, this));

    OT_ASSERT(contact_thread_)
}

bool ActivityThread::check_draft(const ActivityThreadRowID& id) const
{
    const auto& taskID = std::get<3>(id);
    [[maybe_unused]] const auto [status, messageID] =
        api_.OTX().MessageStatus(taskID);

    switch (status) {
        case ThreadStatus::RUNNING:
        case ThreadStatus::FINISHED_SUCCESS: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Message sent successfully.")
                .Flush();

            return true;
        } break;
        case ThreadStatus::FINISHED_FAILED: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to send message.")
                .Flush();
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
    LogOutput(OT_METHOD)(__FUNCTION__)(": Checking ")(draft_tasks_.size())(
        " pending sends.")
        .Flush();
    std::set<ActivityThreadRowID> deleted{};

    for (const auto& draftID : draft_tasks_) {
        if (check_draft(draftID)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Removing successfully sent draft.")
                .Flush();
            Lock lock(lock_);
            delete_item(lock, draftID);
            deleted.emplace(draftID);
        } else {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Keeping pending send.")
                .Flush();
        }
    }

    for (const auto& id : deleted) { draft_tasks_.erase(id); }
}

std::string ActivityThread::comma(const std::set<std::string>& list) const
{
    std::ostringstream stream;

    for (const auto& item : list) {
        stream << item;
        stream << ", ";
    }

    std::string output = stream.str();

    if (0 < output.size()) { output.erase(output.size() - 2, 2); }

    return output;
}

void ActivityThread::construct_row(
    const ActivityThreadRowID& id,
    const ActivityThreadSortKey& index,
    const CustomData& custom) const
{
    names_.emplace(id, index);
    const auto& box = std::get<1>(id);

    switch (box) {
        case StorageBox::MAILINBOX:
        case StorageBox::MAILOUTBOX: {
            items_[index].emplace(
                id,
                Factory::MailItem(
                    *this, api_, publisher_, nym_id_, id, index, custom));
        } break;
        case StorageBox::DRAFT: {
            items_[index].emplace(
                id,
                Factory::MailItem(
                    *this,
                    api_,
                    publisher_,
                    nym_id_,
                    id,
                    index,
                    custom,
                    false,
                    true));
        } break;
        case StorageBox::INCOMINGCHEQUE:
        case StorageBox::OUTGOINGCHEQUE: {
            items_[index].emplace(
                id,
                Factory::PaymentItem(
                    *this, api_, publisher_, nym_id_, id, index, custom));
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
        auto name = api_.Contacts().ContactName(contactID);

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
    wait_for_startup();

    if (1 != participants_.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Wrong number of participants (")(
            participants_.size())(").")
            .Flush();

        return;
    }

    auto contact = api_.Contacts().Contact(*participants_.cbegin());
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

    LogDetail(OT_METHOD)(__FUNCTION__)(": Loading ")(thread.item().size())(
        " items.")
        .Flush();

    for (const auto& item : thread.item()) { process_item(item); }

    startup_complete_->On();
}

void ActivityThread::new_thread()
{
    participants_.emplace(threadID_);
    UpdateNotify();
    startup_complete_->On();
}

std::string ActivityThread::Participants() const
{
    Lock lock(lock_);
    std::set<std::string> ids{};

    for (const auto& id : participants_) { ids.emplace(id->str()); }

    return comma(ids);
}

std::string ActivityThread::PaymentCode(
    const proto::ContactItemType currency) const
{
    Lock lock(contact_lock_);

    if (contact_) { return contact_->PaymentCode(currency); }

    return {};
}

ActivityThreadRowID ActivityThread::process_item(
    const proto::StorageThreadItem& item)
{
    const ActivityThreadRowID id{Identifier::Factory(item.id()),
                                 static_cast<StorageBox>(item.box()),
                                 Identifier::Factory(item.account()),
                                 0};
    const ActivityThreadSortKey key{std::chrono::seconds(item.time()),
                                    item.index()};
    const CustomData custom{new std::string};
    add_item(id, key, custom);

    return id;
}

void ActivityThread::process_thread(const network::zeromq::Message& message)
{
    wait_for_startup();
    check_drafts();

    OT_ASSERT(1 == message.Body().size());

    const std::string id(*message.Body().begin());
    const auto threadID = Identifier::Factory(id);

    OT_ASSERT(false == threadID->empty())

    if (threadID_ != threadID) { return; }

    const auto thread = api_.Activity().Thread(nym_id_, threadID_);

    OT_ASSERT(thread)

    std::set<ActivityThreadRowID> active{};

    for (const auto& item : thread->item()) {
        const auto id = process_item(item);
        active.emplace(id);
    }

    delete_inactive(active);
}

bool ActivityThread::same(
    const ActivityThreadRowID& lhs,
    const ActivityThreadRowID& rhs) const
{
    const auto& [lID, lBox, lAccount, lTask] = lhs;
    const auto& [rID, rBox, rAccount, rTask] = rhs;
    const bool sameID = (lID->str() == rID->str());
    const bool sameBox = (lBox == rBox);
    const bool sameAccount = (lAccount->str() == rAccount->str());
    const bool sametask = (lTask == rTask);

    return sameID && sameBox && sameAccount && sametask;
}

bool ActivityThread::SendDraft() const
{
    eLock draftLock(draft_lock_);

    if (draft_.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": No draft message to send.")
            .Flush();

        return false;
    }

    if (participants_.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": No recipients.").Flush();

        return false;
    }

    if (1 < participants_.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Sending to multiple recipient not yet supported.")
            .Flush();

        return false;
    }

    const auto task =
        api_.OTX().MessageContact(nym_id_, *participants_.begin(), draft_);
    const auto taskID = std::get<0>(task);

    if (0 == taskID) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to queue message for sending.")
            .Flush();

        return false;
    }

    const ActivityThreadRowID id{Identifier::Factory(std::to_string(taskID)),
                                 StorageBox::DRAFT,
                                 Identifier::Factory(),
                                 taskID};
    const ActivityThreadSortKey key{std::chrono::system_clock::now(), 0};
    draft_tasks_.insert(id);
    const CustomData custom{new std::string(draft_)};
    draft_.clear();
    const_cast<ActivityThread&>(*this).add_item(id, key, custom);

    return true;
}

bool ActivityThread::SetDraft(const std::string& draft) const
{
    if (draft.empty()) { return false; }

    eLock lock(draft_lock_);
    draft_ = draft;

    return true;
}

void ActivityThread::startup()
{
    const auto thread = api_.Activity().Thread(nym_id_, threadID_);

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
    for (auto& it : listeners_) { delete it.second; }

    if (contact_thread_ && contact_thread_->joinable()) {
        contact_thread_->join();
        contact_thread_.reset();
    }
}
}  // namespace opentxs::ui::implementation
