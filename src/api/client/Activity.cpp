// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Activity.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Workflow.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/contact/Contact.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/core/contract/peer/PeerObject.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/PublishSocket.hpp"
#include "opentxs/Types.hpp"

#include "InternalClient.hpp"

#include <map>
#include <mutex>
#include <thread>

#include "Activity.hpp"

#define OT_METHOD "opentxs::api::implementation::Activity::"

namespace opentxs
{
api::client::internal::Activity* Factory::Activity(
    const api::Core& api,
    const api::client::Contacts& contact)
{
    return new api::client::implementation::Activity(api, contact);
}
}  // namespace opentxs

namespace opentxs::api::client::implementation
{
Activity::Activity(const api::Core& api, const client::Contacts& contact)
    : api_(api)
    , contact_(contact)
    , mail_cache_lock_()
    , mail_cache_()
    , publisher_lock_()
    , thread_publishers_()
{
    // WARNING: do not access api_.Wallet() during construction
}

void Activity::activity_preload_thread(
    const OTIdentifier nym,
    const std::size_t count) const
{
    const std::string nymID = nym->str();
    auto threads = api_.Storage().ThreadList(nymID, false);

    for (const auto& it : threads) {
        const auto& threadID = it.first;
        thread_preload_thread(nymID, threadID, 0, count);
    }
}

bool Activity::AddBlockchainTransaction(
    const Identifier& nymID,
    const Identifier& threadID,
    const StorageBox box,
    const proto::BlockchainTransaction& transaction) const
{
    eLock lock(shared_lock_);
    const std::string sNymID = nymID.str();
    const std::string sthreadID = threadID.str();
    const auto threadList = api_.Storage().ThreadList(sNymID, false);
    bool threadExists = false;

    for (const auto it : threadList) {
        const auto& id = it.first;

        if (id == sthreadID) {
            threadExists = true;
            break;
        }
    }

    if (false == threadExists) {
        api_.Storage().CreateThread(sNymID, sthreadID, {sthreadID});
    }

    const bool saved = api_.Storage().Store(
        sNymID, sthreadID, transaction.txid(), transaction.time(), {}, {}, box);

    if (saved) { publish(nymID, sthreadID); }

    return saved;
}

bool Activity::AddPaymentEvent(
    const Identifier& nymID,
    const Identifier& threadID,
    const StorageBox type,
    const Identifier& itemID,
    const Identifier& workflowID,
    std::chrono::time_point<std::chrono::system_clock> time) const
{
    eLock lock(shared_lock_);
    const std::string sNymID = nymID.str();
    const std::string sthreadID = threadID.str();
    const auto threadList = api_.Storage().ThreadList(sNymID, false);
    bool threadExists = false;

    for (const auto it : threadList) {
        const auto& id = it.first;

        if (id == sthreadID) {
            threadExists = true;
            break;
        }
    }

    if (false == threadExists) {
        api_.Storage().CreateThread(sNymID, sthreadID, {sthreadID});
    }

    const bool saved = api_.Storage().Store(
        sNymID,
        sthreadID,
        itemID.str(),
        std::chrono::system_clock::to_time_t(time),
        {},
        {},
        type,
        workflowID.str());

    if (saved) { publish(nymID, sthreadID); }

    return saved;
}

Activity::ChequeData Activity::Cheque(
    const Identifier& nym,
    [[maybe_unused]] const std::string& id,
    const std::string& workflowID) const
{
    ChequeData output;
    auto& [cheque, contract] = output;
    auto [type, state] =
        api_.Storage().PaymentWorkflowState(nym.str(), workflowID);
    [[maybe_unused]] const auto& notUsed = state;

    switch (type) {
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE:
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGINVOICE:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGINVOICE: {
        } break;

        case proto::PAYMENTWORKFLOWTYPE_ERROR:
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGTRANSFER:
        case proto::PAYMENTWORKFLOWTYPE_INTERNALTRANSFER:
        default: {
            otErr << OT_METHOD << __FUNCTION__ << ": Wrong workflow type"
                  << std::endl;

            return output;
        }
    }

    std::shared_ptr<proto::PaymentWorkflow> workflow{nullptr};
    const auto loaded = api_.Storage().Load(nym.str(), workflowID, workflow);

    if (false == loaded) {
        otErr << OT_METHOD << __FUNCTION__ << ": Workflow " << workflowID
              << " for nym " << nym.str() << " can not be loaded" << std::endl;

        return output;
    }

    OT_ASSERT(workflow)

    auto instantiated = client::Workflow::InstantiateCheque(api_, *workflow);
    cheque.reset(std::get<1>(instantiated).release());

    OT_ASSERT(cheque)

    const auto& unit = cheque->GetInstrumentDefinitionID();
    contract = api_.Wallet().UnitDefinition(unit);

    if (false == bool(contract)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to load unit definition contract" << std::endl;
    }

    return output;
}

Activity::TransferData Activity::Transfer(
    const Identifier& nym,
    [[maybe_unused]] const std::string& id,
    const std::string& workflowID) const
{
    TransferData output;
    auto& [transfer, contract] = output;
    auto [type, state] =
        api_.Storage().PaymentWorkflowState(nym.str(), workflowID);
    [[maybe_unused]] const auto& notUsed = state;

    switch (type) {
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGTRANSFER:
        case proto::PAYMENTWORKFLOWTYPE_INTERNALTRANSFER: {
        } break;

        case proto::PAYMENTWORKFLOWTYPE_ERROR:
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE:
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGINVOICE:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGINVOICE:
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Wrong workflow type").Flush();

            return output;
        }
    }

    std::shared_ptr<proto::PaymentWorkflow> workflow{nullptr};
    const auto loaded = api_.Storage().Load(nym.str(), workflowID, workflow);

    if (false == loaded) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Workflow ")(workflowID)(
            " for nym ")(nym)(" can not be loaded")
            .Flush();

        return output;
    }

    OT_ASSERT(workflow)

    auto instantiated = client::Workflow::InstantiateTransfer(api_, *workflow);
    transfer.reset(std::get<1>(instantiated).release());

    OT_ASSERT(transfer)

    if (0 == workflow->account_size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Workflow does not list any accounts.")
            .Flush();

        return output;
    }

    const auto unit = api_.Storage().AccountContract(
        Identifier::Factory(workflow->account(0)));

    if (unit->empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to calculate unit definition id.")
            .Flush();

        return output;
    }
    contract = api_.Wallet().UnitDefinition(unit);

    if (false == bool(contract)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to load unit definition contract" << std::endl;
    }

    return output;
}

const opentxs::network::zeromq::PublishSocket& Activity::get_publisher(
    const Identifier& nymID) const
{
    std::string endpoint{};

    return get_publisher(nymID, endpoint);
}

const opentxs::network::zeromq::PublishSocket& Activity::get_publisher(
    const Identifier& nymID,
    std::string& endpoint) const
{
    endpoint = api_.Endpoints().ThreadUpdate(nymID.str());
    Lock lock(publisher_lock_);
    auto it = thread_publishers_.find(nymID);

    if (thread_publishers_.end() != it) { return it->second; }

    const auto& [publisher, inserted] =
        thread_publishers_.emplace(nymID, api_.ZeroMQ().PublishSocket());

    OT_ASSERT(inserted)

    auto& output = publisher->second.get();
    output.Start(endpoint);
    LogDetail(OT_METHOD)(__FUNCTION__)(": Publisher started on ")(endpoint)
        .Flush();

    return output;
}

bool Activity::MoveIncomingBlockchainTransaction(
    const Identifier& nymID,
    const Identifier& fromThreadID,
    const Identifier& toThreadID,
    const std::string& txid) const
{
    return api_.Storage().MoveThreadItem(
        nymID.str(), fromThreadID.str(), toThreadID.str(), txid);
}

std::unique_ptr<Message> Activity::Mail(
    const Identifier& nym,
    const Identifier& id,
    const StorageBox& box) const
{
    std::string raw, alias;
    const bool loaded =
        api_.Storage().Load(nym.str(), id.str(), box, raw, alias, true);

    std::unique_ptr<Message> output;

    if (!loaded) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to load Message"
              << std::endl;

        return output;
    }

    if (raw.empty()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Empty message" << std::endl;

        return output;
    }

    output.reset(api_.Factory().Message().release());

    OT_ASSERT(output);

    if (false == output->LoadContractFromString(String::Factory(raw.c_str()))) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to deserialized Message"
              << std::endl;

        output.reset();
    }

    return output;
}

std::string Activity::Mail(
    const Identifier& nym,
    const Message& mail,
    const StorageBox box) const
{
    const std::string nymID = nym.str();
    auto id = Identifier::Factory();
    mail.CalculateContractID(id);
    const std::string output = id->str();
    const auto data = String::Factory(mail);
    std::string participantNymID;
    const auto localName = String::Factory(nym);

    if (localName->Compare(mail.m_strNymID2)) {
        // This is an incoming message. The contact id is the sender's id.
        participantNymID = mail.m_strNymID->Get();
    } else {
        // This is an outgoing message. The contact id is the recipient's id.
        participantNymID = mail.m_strNymID2->Get();
    }

    const auto contact = nym_to_contact(participantNymID);

    OT_ASSERT(contact);

    eLock lock(shared_lock_);
    std::string alias = contact->Label();
    const std::string contactID = contact->ID().str();
    const auto& threadID = contactID;
    const auto threadList = api_.Storage().ThreadList(nymID, false);
    bool threadExists = false;

    for (const auto it : threadList) {
        const auto& id = it.first;

        if (id == threadID) {
            threadExists = true;
            break;
        }
    }

    if (false == threadExists) {
        api_.Storage().CreateThread(nymID, threadID, {contactID});
    }

    const bool saved = api_.Storage().Store(
        localName->Get(),
        threadID,
        output,
        mail.m_lTime,
        alias,
        data->Get(),
        box);

    if (saved) {
        std::thread preload(
            &Activity::preload,
            this,
            Identifier::Factory(nym),
            Identifier::Factory(id),
            box);
        preload.detach();
        publish(nym, threadID);

        return output;
    }

    return "";
}

ObjectList Activity::Mail(const Identifier& nym, const StorageBox box) const
{
    return api_.Storage().NymBoxList(nym.str(), box);
}

bool Activity::MailRemove(
    const Identifier& nym,
    const Identifier& id,
    const StorageBox box) const
{
    const std::string nymid = nym.str();
    const std::string mail = id.str();

    return api_.Storage().RemoveNymBoxItem(nymid, box, mail);
}

std::shared_ptr<const std::string> Activity::MailText(
    const Identifier& nymID,
    const Identifier& id,
    const StorageBox& box) const
{
    Lock lock(mail_cache_lock_);
    auto it = mail_cache_.find(id);
    lock.unlock();

    if (mail_cache_.end() != it) { return it->second; }

    preload(nymID, id, box);
    lock.lock();
    it = mail_cache_.find(id);
    lock.unlock();

    if (mail_cache_.end() == it) { return {}; }

    return it->second;
}

bool Activity::MarkRead(
    const Identifier& nymId,
    const Identifier& threadId,
    const Identifier& itemId) const
{
    const std::string nym = nymId.str();
    const std::string thread = threadId.str();
    const std::string item = itemId.str();

    return api_.Storage().SetReadState(nym, thread, item, false);
}

bool Activity::MarkUnread(
    const Identifier& nymId,
    const Identifier& threadId,
    const Identifier& itemId) const
{
    const std::string nym = nymId.str();
    const std::string thread = threadId.str();
    const std::string item = itemId.str();

    return api_.Storage().SetReadState(nym, thread, item, true);
}

void Activity::MigrateLegacyThreads() const
{
    eLock lock(shared_lock_);
    std::set<std::string> contacts{};

    for (const auto& it : contact_.ContactList()) { contacts.insert(it.first); }

    const auto nymlist = api_.Storage().NymList();

    for (const auto& it1 : nymlist) {
        const auto& nymID = it1.first;
        const auto threadList = api_.Storage().ThreadList(nymID, false);

        for (const auto& it2 : threadList) {
            const auto& originalThreadID = it2.first;
            const bool isContactID = (1 == contacts.count(originalThreadID));

            if (isContactID) { continue; }

            auto contactID =
                contact_.ContactID(Identifier::Factory(originalThreadID));

            if (false == contactID->empty()) {
                api_.Storage().RenameThread(
                    nymID, originalThreadID, contactID->str());
            } else {
                std::shared_ptr<proto::StorageThread> thread;
                api_.Storage().Load(nymID, originalThreadID, thread);

                OT_ASSERT(thread);

                const auto nymCount = thread->participant().size();

                if (1 == nymCount) {
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
                    auto paymentCode = api_.Factory().PaymentCode("");
#endif
                    auto newContact = contact_.NewContact(
                        "",
                        Identifier::Factory(originalThreadID)
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
                            ,
                        paymentCode
#endif
                    );

                    OT_ASSERT(newContact);

                    api_.Storage().RenameThread(
                        nymID, originalThreadID, newContact->ID().str());
                } else {
                    // Multi-party chats were not implemented prior to the
                    // update to contact IDs, so there is no need to handle
                    // this case
                }
            }
        }
    }
}

std::shared_ptr<const Contact> Activity::nym_to_contact(
    const std::string& id) const
{
    const auto nymID = Identifier::Factory(id);
    const auto contactID = contact_.NymToContact(nymID);

    return contact_.Contact(contactID);
}

std::shared_ptr<const std::string> Activity::PaymentText(
    const Identifier& nym,
    const std::string& id,
    const std::string& workflowID) const
{
    std::shared_ptr<std::string> output;
    auto [type, state] =
        api_.Storage().PaymentWorkflowState(nym.str(), workflowID);
    [[maybe_unused]] const auto& notUsed = state;

    switch (type) {
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE: {
            output.reset(new std::string("Sent cheque"));
        } break;
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE: {
            output.reset(new std::string("Received cheque"));
        } break;
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER: {
            output.reset(new std::string("Sent transfer"));
        } break;
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGTRANSFER: {
            output.reset(new std::string("Received transfer"));
        } break;
        case proto::PAYMENTWORKFLOWTYPE_INTERNALTRANSFER: {
            output.reset(new std::string("Internal transfer"));
        } break;

        case proto::PAYMENTWORKFLOWTYPE_ERROR:
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGINVOICE:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGINVOICE:
        default: {

            return output;
        }
    }

    std::shared_ptr<proto::PaymentWorkflow> workflow{nullptr};
    const auto loaded = api_.Storage().Load(nym.str(), workflowID, workflow);

    if (false == loaded) {
        otErr << OT_METHOD << __FUNCTION__ << ": Workflow " << workflowID
              << " for nym " << nym.str() << " can not be loaded" << std::endl;

        return output;
    }

    OT_ASSERT(workflow)

    switch (type) {
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE:
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGINVOICE:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGINVOICE: {
            auto chequeData = Cheque(nym, id, workflowID);
            const auto& [cheque, contract] = chequeData;

            OT_ASSERT(cheque)

            if (contract) {
                std::string amount{};
                const bool haveAmount = contract->FormatAmountLocale(
                    cheque->GetAmount(), amount, ",", ".");

                if (haveAmount) {
                    const std::string text =
                        *output + std::string{" for "} + amount;
                    *output = text;
                }
            }
        } break;

        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGTRANSFER:
        case proto::PAYMENTWORKFLOWTYPE_INTERNALTRANSFER: {
            auto transferData = Transfer(nym, id, workflowID);
            const auto& [transfer, contract] = transferData;

            OT_ASSERT(transfer)

            if (contract) {
                std::string amount{};
                const bool haveAmount = contract->FormatAmountLocale(
                    transfer->GetAmount(), amount, ",", ".");

                if (haveAmount) {
                    const std::string text =
                        *output + std::string{" for "} + amount;
                    *output = text;
                }
            }
        } break;

        case proto::PAYMENTWORKFLOWTYPE_ERROR:
        default: {

            return nullptr;
        }
    }

    return output;
}

void Activity::preload(
    const OTIdentifier nymID,
    const OTIdentifier id,
    const StorageBox box) const
{
    const auto message = Mail(nymID, id, box);

    if (!message) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to load message "
              << String::Factory(id) << std::endl;

        return;
    }

    auto nym = api_.Wallet().Nym(nymID);

    if (false == bool(nym)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to load recipent nym."
              << std::endl;

        return;
    }

    otErr << OT_METHOD << __FUNCTION__ << ": Decrypting message " << id->str()
          << std::endl;
    auto peerObject = PeerObject::Factory(
        contact_, api_.Wallet(), nym, message->m_ascPayload);
    otErr << OT_METHOD << __FUNCTION__ << ": Message " << id->str()
          << " decrypted." << std::endl;

    if (!peerObject) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to instantiate peer object." << std::endl;

        return;
    }

    if (!peerObject->Message()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Peer object does not contain a message." << std::endl;

        return;
    }

    Lock lock(mail_cache_lock_);
    auto& output = mail_cache_[id];
    lock.unlock();
    output.reset(new std::string(*peerObject->Message()));
}

void Activity::PreloadActivity(const Identifier& nymID, const std::size_t count)
    const
{
    std::thread preload(
        &Activity::activity_preload_thread,
        this,
        Identifier::Factory(nymID),
        count);
    preload.detach();
}

void Activity::PreloadThread(
    const Identifier& nymID,
    const Identifier& threadID,
    const std::size_t start,
    const std::size_t count) const
{
    const std::string nym = nymID.str();
    const std::string thread = threadID.str();
    std::thread preload(
        &Activity::thread_preload_thread, this, nym, thread, start, count);
    preload.detach();
}

void Activity::publish(const Identifier& nymID, const std::string& threadID)
    const
{
    auto& publisher = get_publisher(nymID);
    publisher.Publish(threadID);
}

std::shared_ptr<proto::StorageThread> Activity::Thread(
    const Identifier& nymID,
    const Identifier& threadID) const
{
    sLock lock(shared_lock_);
    std::shared_ptr<proto::StorageThread> output;
    api_.Storage().Load(nymID.str(), threadID.str(), output);

    return output;
}

void Activity::thread_preload_thread(
    const std::string nymID,
    const std::string threadID,
    const std::size_t start,
    const std::size_t count) const
{
    std::shared_ptr<proto::StorageThread> thread{};
    const bool loaded = api_.Storage().Load(nymID, threadID, thread);

    if (false == loaded) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to load thread "
              << threadID << " for nym " << nymID << std::endl;

        return;
    }

    const std::size_t size = thread->item_size();
    std::size_t cached{0};

    if (start > size) {
        otErr << OT_METHOD << __FUNCTION__ << ": Error: start larger than size "
              << "(" << std::to_string(start) << "/" << std::to_string(size)
              << ")" << std::endl;

        return;
    }

    for (auto i = (size - start); i > 0; --i) {
        if (cached >= count) { break; }

        const auto& item = thread->item(i - 1);
        const auto& box = static_cast<StorageBox>(item.box());

        switch (box) {
            case StorageBox::MAILINBOX:
            case StorageBox::MAILOUTBOX: {
                otErr << OT_METHOD << __FUNCTION__ << ": Preloading item "
                      << item.id() << " in thread " << threadID << std::endl;
                MailText(
                    Identifier::Factory(nymID),
                    Identifier::Factory(item.id()),
                    box);
                ++cached;
            } break;
            default: {
                continue;
            }
        }
    }
}

std::string Activity::ThreadPublisher(const Identifier& nym) const
{
    std::string endpoint{};
    get_publisher(nym, endpoint);

    return endpoint;
}

ObjectList Activity::Threads(const Identifier& nym, const bool unreadOnly) const
{
    const std::string nymID = nym.str();
    auto output = api_.Storage().ThreadList(nymID, unreadOnly);

    for (auto& it : output) {
        const auto& threadID = it.first;
        auto& label = it.second;

        if (label.empty()) {
            auto contact = contact_.Contact(Identifier::Factory(threadID));

            if (contact) {
                const auto& name = contact->Label();

                if (label != name) {
                    api_.Storage().SetThreadAlias(nymID, threadID, name);
                    label = name;
                }
            }
        }
    }

    return output;
}

std::size_t Activity::UnreadCount(const Identifier& nymId) const
{
    const std::string nym = nymId.str();
    std::size_t output{0};

    const auto& threads = api_.Storage().ThreadList(nym, true);

    for (const auto& it : threads) {
        const auto& threadId = it.first;
        output += api_.Storage().UnreadCount(nym, threadId);
    }

    return output;
}
}  // namespace opentxs::api::client::implementation
