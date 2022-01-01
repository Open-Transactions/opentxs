// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                       // IWYU pragma: associated
#include "1_Internal.hpp"                     // IWYU pragma: associated
#include "api/session/activity/Activity.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#include "Proto.hpp"
#include "internal/api/session/Factory.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Activity.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/api/session/Workflow.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"  // IWYU pragma: keep
#include "opentxs/contact/Contact.hpp"
#include "opentxs/core/Cheque.hpp"  // IWYU pragma: keep
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Item.hpp"  // IWYU pragma: keep
#include "opentxs/core/Message.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/contract/peer/PeerObject.hpp"
#include "opentxs/core/display/Definition.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/otx/client/PaymentWorkflowType.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/WorkType.hpp"
#include "serialization/protobuf/PaymentWorkflow.pb.h"
#include "serialization/protobuf/StorageThread.pb.h"
#include "serialization/protobuf/StorageThreadItem.pb.h"

namespace opentxs::factory
{
auto ActivityAPI(
    const api::Session& api,
    const api::session::Contacts& contact) noexcept
    -> std::unique_ptr<api::session::Activity>
{
    using ReturnType = api::session::imp::Activity;

    return std::make_unique<ReturnType>(api, contact);
}
}  // namespace opentxs::factory

namespace opentxs::api::session::imp
{
Activity::Activity(
    const api::Session& api,
    const session::Contacts& contact) noexcept
    : api_(api)
    , contact_(contact)
    , message_loaded_([&] {
        auto out = api_.Network().ZeroMQ().PublishSocket();
        const auto rc = out->Start(api_.Endpoints().MessageLoaded());

        OT_ASSERT(rc);

        return out;
    }())
    , mail_(api_, message_loaded_)
    , publisher_lock_()
    , thread_publishers_()
    , blockchain_publishers_()
{
    // WARNING: do not access api_.Wallet() during construction
}

auto Activity::activity_preload_thread(
    OTPasswordPrompt reason,
    const OTIdentifier nym,
    const std::size_t count) const noexcept -> void
{
    const std::string nymID = nym->str();
    auto threads = api_.Storage().ThreadList(nymID, false);

    for (const auto& it : threads) {
        const auto& threadID = it.first;
        thread_preload_thread(reason, nymID, threadID, 0, count);
    }
}

#if OT_BLOCKCHAIN
auto Activity::add_blockchain_transaction(
    const eLock& lock,
    const identifier::Nym& nym,
    const blockchain::block::bitcoin::Transaction& transaction) const noexcept
    -> bool
{
    const auto incoming = transaction.AssociatedRemoteContacts(contact_, nym);
    LogTrace()(OT_PRETTY_CLASS())("transaction ")(transaction.ID().asHex())(
        " is associated  with ")(incoming.size())(" contacts")
        .Flush();
    const auto existing =
        api_.Storage().BlockchainThreadMap(nym, transaction.ID());
    auto added = std::pmr::vector<OTIdentifier>{};
    auto removed = std::pmr::vector<OTIdentifier>{};
    std::set_difference(
        std::begin(incoming),
        std::end(incoming),
        std::begin(existing),
        std::end(existing),
        std::back_inserter(added));
    std::set_difference(
        std::begin(existing),
        std::end(existing),
        std::begin(incoming),
        std::end(incoming),
        std::back_inserter(removed));
    auto output{true};
    const auto& txid = transaction.ID();
    const auto chains = transaction.Chains();

    for (const auto& thread : added) {
        if (thread->empty()) { continue; }

        const auto sThreadID = thread->str();

        if (verify_thread_exists(nym.str(), sThreadID)) {
            auto saved{true};
            std::for_each(
                std::begin(chains), std::end(chains), [&](const auto& chain) {
                    saved &= api_.Storage().Store(
                        nym, thread, chain, txid, transaction.Timestamp());
                });

            if (saved) { publish(nym, thread); }

            output &= saved;
        } else {
            output = false;
        }
    }

    for (const auto& thread : removed) {
        if (thread->empty()) { continue; }

        auto saved{true};
        const auto chains = transaction.Chains();
        std::for_each(
            std::begin(chains), std::end(chains), [&](const auto& chain) {
                saved &= api_.Storage().RemoveBlockchainThreadItem(
                    nym, thread, chain, txid);
            });

        if (saved) { publish(nym, thread); }

        output &= saved;
    }

    if (0 == incoming.size()) {
        api_.Storage().UnaffiliatedBlockchainTransaction(nym, txid);
    }

    std::for_each(std::begin(chains), std::end(chains), [&](const auto& chain) {
        get_blockchain(lock, nym).Send([&] {
            auto out = opentxs::network::zeromq::tagged_message(
                WorkType::BlockchainNewTransaction);
            out.AddFrame(txid);
            out.AddFrame(chain);

            return out;
        }());
    });

    return output;
}
#endif  // OT_BLOCKCHAIN

auto Activity::AddBlockchainTransaction(
    const blockchain::block::bitcoin::Transaction& transaction) const noexcept
    -> bool
{
#if OT_BLOCKCHAIN
    auto lock = eLock(shared_lock_);

    for (const auto& nym : transaction.AssociatedLocalNyms()) {
        LogTrace()(OT_PRETTY_CLASS())("blockchain transaction ")(
            transaction.ID().asHex())(" is relevant to local nym ")(
            nym->asHex())
            .Flush();

        OT_ASSERT(false == nym->empty());

        if (false == add_blockchain_transaction(lock, nym, transaction)) {
            return false;
        }
    }

    return true;
#else
    return false;
#endif  // OT_BLOCKCHAIN
}

auto Activity::AddPaymentEvent(
    const identifier::Nym& nymID,
    const Identifier& threadID,
    const StorageBox type,
    const Identifier& itemID,
    const Identifier& workflowID,
    Time time) const noexcept -> bool
{
    auto lock = eLock(shared_lock_);
    const std::string sNymID = nymID.str();
    const std::string sthreadID = threadID.str();

    if (false == verify_thread_exists(sNymID, sthreadID)) { return false; }

    const bool saved = api_.Storage().Store(
        sNymID,
        sthreadID,
        itemID.str(),
        Clock::to_time_t(time),
        {},
        {},
        type,
        workflowID.str());

    if (saved) { publish(nymID, threadID); }

    return saved;
}

auto Activity::Cheque(
    const identifier::Nym& nym,
    [[maybe_unused]] const std::string& id,
    const std::string& workflowID) const noexcept -> Activity::ChequeData
{
    auto output = ChequeData{nullptr, api_.Factory().UnitDefinition()};
    auto& [cheque, contract] = output;
    auto [type, state] =
        api_.Storage().PaymentWorkflowState(nym.str(), workflowID);
    [[maybe_unused]] const auto& notUsed = state;

    switch (type) {
        case otx::client::PaymentWorkflowType::OutgoingCheque:
        case otx::client::PaymentWorkflowType::IncomingCheque:
        case otx::client::PaymentWorkflowType::OutgoingInvoice:
        case otx::client::PaymentWorkflowType::IncomingInvoice: {
        } break;

        case otx::client::PaymentWorkflowType::Error:
        case otx::client::PaymentWorkflowType::OutgoingTransfer:
        case otx::client::PaymentWorkflowType::IncomingTransfer:
        case otx::client::PaymentWorkflowType::InternalTransfer:
        default: {
            LogError()(OT_PRETTY_CLASS())("Wrong workflow type.").Flush();

            return output;
        }
    }

    auto workflow = proto::PaymentWorkflow{};

    if (false == api_.Storage().Load(nym.str(), workflowID, workflow)) {
        LogError()(OT_PRETTY_CLASS())("Workflow ")(workflowID)(" for nym ")(
            nym)(" can not be loaded.")
            .Flush();

        return output;
    }

    auto instantiated = session::Workflow::InstantiateCheque(api_, workflow);
    cheque.reset(std::get<1>(instantiated).release());

    OT_ASSERT(cheque)

    const auto& unit = cheque->GetInstrumentDefinitionID();

    try {
        contract = api_.Wallet().UnitDefinition(unit);
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())(
            "Unable to load unit definition contract.")
            .Flush();
    }

    return output;
}

auto Activity::Transfer(
    const identifier::Nym& nym,
    [[maybe_unused]] const std::string& id,
    const std::string& workflowID) const noexcept -> Activity::TransferData
{
    auto output = TransferData{nullptr, api_.Factory().UnitDefinition()};
    auto& [transfer, contract] = output;
    auto [type, state] =
        api_.Storage().PaymentWorkflowState(nym.str(), workflowID);

    switch (type) {
        case otx::client::PaymentWorkflowType::OutgoingTransfer:
        case otx::client::PaymentWorkflowType::IncomingTransfer:
        case otx::client::PaymentWorkflowType::InternalTransfer: {
        } break;

        case otx::client::PaymentWorkflowType::Error:
        case otx::client::PaymentWorkflowType::OutgoingCheque:
        case otx::client::PaymentWorkflowType::IncomingCheque:
        case otx::client::PaymentWorkflowType::OutgoingInvoice:
        case otx::client::PaymentWorkflowType::IncomingInvoice:
        default: {
            LogError()(OT_PRETTY_CLASS())("Wrong workflow type").Flush();

            return output;
        }
    }

    auto workflow = proto::PaymentWorkflow{};

    if (false == api_.Storage().Load(nym.str(), workflowID, workflow)) {
        LogError()(OT_PRETTY_CLASS())("Workflow ")(workflowID)(" for nym ")(
            nym)(" can not be loaded")
            .Flush();

        return output;
    }

    auto instantiated = session::Workflow::InstantiateTransfer(api_, workflow);
    transfer.reset(std::get<1>(instantiated).release());

    OT_ASSERT(transfer)

    if (0 == workflow.account_size()) {
        LogError()(OT_PRETTY_CLASS())("Workflow does not list any accounts.")
            .Flush();

        return output;
    }

    const auto unit = api_.Storage().AccountContract(
        Identifier::Factory(workflow.account(0)));

    if (unit->empty()) {
        LogError()(OT_PRETTY_CLASS())("Unable to calculate unit definition id.")
            .Flush();

        return output;
    }

    try {
        contract = api_.Wallet().UnitDefinition(unit);
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())(
            "Unable to load unit definition contract.")
            .Flush();
    }

    return output;
}

auto Activity::get_publisher(const identifier::Nym& nymID) const noexcept
    -> const opentxs::network::zeromq::socket::Publish&
{
    std::string endpoint{};

    return get_publisher(nymID, endpoint);
}

#if OT_BLOCKCHAIN
auto Activity::get_blockchain(const eLock&, const identifier::Nym& nymID)
    const noexcept -> const opentxs::network::zeromq::socket::Publish&
{
    auto it = blockchain_publishers_.find(nymID);

    if (blockchain_publishers_.end() != it) { return it->second; }

    const auto endpoint = api_.Endpoints().BlockchainTransactions(nymID);
    const auto& [publisher, inserted] =
        blockchain_publishers_.emplace(nymID, start_publisher(endpoint));

    OT_ASSERT(inserted);

    return publisher->second.get();
}
#endif  // OT_BLOCKCHAIN

auto Activity::get_publisher(
    const identifier::Nym& nymID,
    std::string& endpoint) const noexcept
    -> const opentxs::network::zeromq::socket::Publish&
{
    endpoint = api_.Endpoints().ThreadUpdate(nymID.str());
    Lock lock(publisher_lock_);
    auto it = thread_publishers_.find(nymID);

    if (thread_publishers_.end() != it) { return it->second; }

    const auto& [publisher, inserted] =
        thread_publishers_.emplace(nymID, start_publisher(endpoint));

    OT_ASSERT(inserted);

    return publisher->second.get();
}

auto Activity::Mail(
    const identifier::Nym& nym,
    const Message& mail,
    const StorageBox box,
    const std::string& text) const noexcept -> std::string
{
    const auto nymID = nym.str();
    auto id = api_.Factory().Identifier();
    mail.CalculateContractID(id);
    const auto itemID = id->str();
    mail_.CacheText(nym, id, box, text);
    const auto data = std::string{String::Factory(mail)->Get()};
    const auto localName = String::Factory(nym);
    const auto participantNymID = [&]() -> std::string {
        if (localName->Compare(mail.m_strNymID2)) {
            // This is an incoming message. The contact id is the sender's id.

            return mail.m_strNymID->Get();
        } else {
            // This is an outgoing message. The contact id is the recipient's
            // id.

            return mail.m_strNymID2->Get();
        }
    }();
    const auto contact = nym_to_contact(participantNymID);

    OT_ASSERT(contact);

    auto lock = eLock(shared_lock_);
    const auto& alias = contact->Label();
    const auto& contactID = contact->ID();
    const auto threadID = contactID.str();

    if (false == verify_thread_exists(nymID, threadID)) { return {}; }

    const bool saved = api_.Storage().Store(
        localName->Get(), threadID, itemID, mail.m_lTime, alias, data, box);

    if (saved) {
        publish(nym, contactID);

        return itemID;
    }

    return "";
}

auto Activity::Mail(const identifier::Nym& nym, const StorageBox box)
    const noexcept -> ObjectList
{
    return api_.Storage().NymBoxList(nym.str(), box);
}

auto Activity::Mail(
    const identifier::Nym& nym,
    const Message& mail,
    const StorageBox box,
    const PeerObject& text) const noexcept -> std::string
{
    return Mail(nym, mail, box, [&]() -> std::string {
        if (auto& out = text.Message(); out) {

            return *out;
        } else {

            return {};
        }
    }());
}

auto Activity::MailRemove(
    const identifier::Nym& nym,
    const Identifier& id,
    const StorageBox box) const noexcept -> bool
{
    const std::string nymid = nym.str();
    const std::string mail = id.str();

    return api_.Storage().RemoveNymBoxItem(nymid, box, mail);
}

auto Activity::MarkRead(
    const identifier::Nym& nymId,
    const Identifier& threadId,
    const Identifier& itemId) const noexcept -> bool
{
    const std::string nym = nymId.str();
    const std::string thread = threadId.str();
    const std::string item = itemId.str();

    return api_.Storage().SetReadState(nym, thread, item, false);
}

auto Activity::MarkUnread(
    const identifier::Nym& nymId,
    const Identifier& threadId,
    const Identifier& itemId) const noexcept -> bool
{
    const std::string nym = nymId.str();
    const std::string thread = threadId.str();
    const std::string item = itemId.str();

    return api_.Storage().SetReadState(nym, thread, item, true);
}

auto Activity::nym_to_contact(const std::string& id) const noexcept
    -> std::shared_ptr<const contact::Contact>
{
    const auto nymID = identifier::Nym::Factory(id);
    const auto contactID = contact_.NymToContact(nymID);

    return contact_.Contact(contactID);
}

auto Activity::PaymentText(
    const identifier::Nym& nym,
    const std::string& id,
    const std::string& workflowID) const noexcept
    -> std::shared_ptr<const std::string>
{
    std::shared_ptr<std::string> output;
    auto [type, state] =
        api_.Storage().PaymentWorkflowState(nym.str(), workflowID);
    [[maybe_unused]] const auto& notUsed = state;

    switch (type) {
        case otx::client::PaymentWorkflowType::OutgoingCheque: {
            output.reset(new std::string("Sent cheque"));
        } break;
        case otx::client::PaymentWorkflowType::IncomingCheque: {
            output.reset(new std::string("Received cheque"));
        } break;
        case otx::client::PaymentWorkflowType::OutgoingTransfer: {
            output.reset(new std::string("Sent transfer"));
        } break;
        case otx::client::PaymentWorkflowType::IncomingTransfer: {
            output.reset(new std::string("Received transfer"));
        } break;
        case otx::client::PaymentWorkflowType::InternalTransfer: {
            output.reset(new std::string("Internal transfer"));
        } break;

        case otx::client::PaymentWorkflowType::Error:
        case otx::client::PaymentWorkflowType::OutgoingInvoice:
        case otx::client::PaymentWorkflowType::IncomingInvoice:
        default: {

            return std::move(output);
        }
    }

    auto workflow = proto::PaymentWorkflow{};

    if (false == api_.Storage().Load(nym.str(), workflowID, workflow)) {
        LogError()(OT_PRETTY_CLASS())("Workflow ")(workflowID)(" for nym ")(
            nym)(" can not be loaded.")
            .Flush();

        return std::move(output);
    }

    switch (type) {
        case otx::client::PaymentWorkflowType::OutgoingCheque:
        case otx::client::PaymentWorkflowType::IncomingCheque:
        case otx::client::PaymentWorkflowType::OutgoingInvoice:
        case otx::client::PaymentWorkflowType::IncomingInvoice: {
            auto chequeData = Cheque(nym, id, workflowID);
            const auto& [cheque, contract] = chequeData;

            OT_ASSERT(cheque)

            if (0 < contract->Version()) {
                const auto& definition =
                    opentxs::display::GetDefinition(contract->UnitOfAccount());
                const auto amount = definition.Format(cheque->GetAmount());

                if (0 < amount.size()) {
                    const std::string text =
                        *output + std::string{" for "} + amount;
                    *output = text;
                }
            }
        } break;

        case otx::client::PaymentWorkflowType::OutgoingTransfer:
        case otx::client::PaymentWorkflowType::IncomingTransfer:
        case otx::client::PaymentWorkflowType::InternalTransfer: {
            auto transferData = Transfer(nym, id, workflowID);
            const auto& [transfer, contract] = transferData;

            OT_ASSERT(transfer)

            if (0 < contract->Version()) {
                const auto& definition =
                    display::GetDefinition(contract->UnitOfAccount());
                const auto amount = definition.Format(transfer->GetAmount());

                if (0 < amount.size()) {
                    const std::string text =
                        *output + std::string{" for "} + amount;
                    *output = text;
                }
            }
        } break;

        case otx::client::PaymentWorkflowType::Error:
        default: {

            return nullptr;
        }
    }

    return std::move(output);
}

auto Activity::PreloadActivity(
    const identifier::Nym& nymID,
    const std::size_t count,
    const PasswordPrompt& reason) const noexcept -> void
{
    std::thread preload(
        &Activity::activity_preload_thread,
        this,
        OTPasswordPrompt{reason},
        Identifier::Factory(nymID),
        count);
    preload.detach();
}

auto Activity::PreloadThread(
    const identifier::Nym& nymID,
    const Identifier& threadID,
    const std::size_t start,
    const std::size_t count,
    const PasswordPrompt& reason) const noexcept -> void
{
    const std::string nym = nymID.str();
    const std::string thread = threadID.str();
    std::thread preload(
        &Activity::thread_preload_thread,
        this,
        OTPasswordPrompt{reason},
        nym,
        thread,
        start,
        count);
    preload.detach();
}

auto Activity::publish(const identifier::Nym& nymID, const Identifier& threadID)
    const noexcept -> void
{
    auto& socket = get_publisher(nymID);
    socket.Send([&] {
        auto work = opentxs::network::zeromq::tagged_message(
            WorkType::ActivityThreadUpdated);
        work.AddFrame(threadID);

        return work;
    }());
}

auto Activity::start_publisher(const std::string& endpoint) const noexcept
    -> OTZMQPublishSocket
{
    auto output = api_.Network().ZeroMQ().PublishSocket();
    const auto started = output->Start(endpoint);

    OT_ASSERT(started);

    LogDetail()(OT_PRETTY_CLASS())("Publisher started on ")(endpoint).Flush();

    return output;
}

auto Activity::Thread(
    const identifier::Nym& nymID,
    const Identifier& threadID,
    proto::StorageThread& output) const noexcept -> bool
{
    auto lock = sLock(shared_lock_);

    if (false == api_.Storage().Load(nymID.str(), threadID.str(), output)) {
        return false;
    }

    return true;
}

auto Activity::Thread(
    const identifier::Nym& nymID,
    const Identifier& threadID,
    AllocateOutput output) const noexcept -> bool
{
    auto lock = sLock(shared_lock_);

    auto serialized = proto::StorageThread{};
    if (false == api_.Storage().Load(nymID.str(), threadID.str(), serialized)) {
        return false;
    }

    return write(serialized, output);
}

auto Activity::thread_preload_thread(
    OTPasswordPrompt reason,
    const std::string nymID,
    const std::string threadID,
    const std::size_t start,
    const std::size_t count) const noexcept -> void
{
    const auto nym = api_.Factory().NymID(nymID);
    auto thread = proto::StorageThread{};

    if (false == api_.Storage().Load(nymID, threadID, thread)) {
        LogError()(OT_PRETTY_CLASS())("Unable to load thread ")(
            threadID)(" for nym ")(nymID)
            .Flush();

        return;
    }

    const auto size = std::size_t(thread.item_size());
    auto cached = std::size_t{0};

    if (start > size) {
        LogError()(OT_PRETTY_CLASS())("Error: start larger than size (")(
            start)(" / ")(size)(")")
            .Flush();

        return;
    }

    OT_ASSERT((size - start) <= std::numeric_limits<int>::max());

    for (auto i = (size - start); i > 0u; --i) {
        if (cached >= count) { break; }

        const auto& item = thread.item(static_cast<int>(i - 1u));
        const auto& box = static_cast<StorageBox>(item.box());

        switch (box) {
            case StorageBox::MAILINBOX:
            case StorageBox::MAILOUTBOX: {
                LogTrace()(OT_PRETTY_CLASS())("Preloading item ")(item.id())(
                    " in thread ")(threadID)
                    .Flush();
                mail_.GetText(
                    nym,
                    api_.Factory().IdentifierFromBase58(item.id()),
                    box,
                    reason);
                ++cached;
            } break;
            default: {
                continue;
            }
        }
    }
}

auto Activity::ThreadPublisher(const identifier::Nym& nym) const noexcept
    -> std::string
{
    std::string endpoint{};
    get_publisher(nym, endpoint);

    return endpoint;
}

auto Activity::Threads(const identifier::Nym& nym, const bool unreadOnly)
    const noexcept -> ObjectList
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

auto Activity::UnreadCount(const identifier::Nym& nymId) const noexcept
    -> std::size_t
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

auto Activity::verify_thread_exists(
    const std::string& nym,
    const std::string& thread) const noexcept -> bool
{
    const auto list = api_.Storage().ThreadList(nym, false);

    for (const auto& it : list) {
        const auto& id = it.first;

        if (id == thread) { return true; }
    }

    return api_.Storage().CreateThread(nym, thread, {thread});
}

Activity::~Activity() = default;
}  // namespace opentxs::api::session::imp
