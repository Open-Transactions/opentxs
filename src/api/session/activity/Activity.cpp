// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::api::session::Activity

#include "api/session/activity/Activity.hpp"  // IWYU pragma: associated

#include <BlockchainTransaction.pb.h>
#include <PaymentWorkflow.pb.h>
#include <StorageThread.pb.h>
#include <StorageThreadItem.pb.h>
#include <algorithm>
#include <functional>
#include <iterator>
#include <limits>
#include <optional>
#include <thread>
#include <utility>

#include "internal/api/session/Storage.hpp"
#include "internal/blockchain/block/Transaction.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Transaction.hpp"
#include "internal/core/String.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/core/contract/peer/Object.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/message/Message.hpp"
#include "internal/network/zeromq/socket/Publish.hpp"
#include "internal/otx/common/Cheque.hpp"  // IWYU pragma: keep
#include "internal/otx/common/Item.hpp"    // IWYU pragma: keep
#include "internal/otx/common/Message.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Pimpl.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/Network.hpp"
#include "opentxs/api/network/ZeroMQ.hpp"
#include "opentxs/api/session/Activity.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Factory.internal.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/api/session/Wallet.internal.hpp"
#include "opentxs/api/session/Workflow.hpp"
#include "opentxs/api/session/internal.factory.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Transaction.hpp"  // IWYU pragma: keep
#include "opentxs/core/Contact.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/display/Definition.hpp"
#include "opentxs/identifier/Account.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/identifier/UnitDefinition.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/otx/client/PaymentWorkflowType.hpp"  // IWYU pragma: keep
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto ActivityAPI(
    const api::session::Client& api,
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
    const api::session::Client& api,
    const session::Contacts& contact) noexcept
    : api_(api)
    , contact_(contact)
    , message_loaded_([&] {
        auto out = api_.Network().ZeroMQ().Context().Internal().PublishSocket();
        const auto rc = out->Start(api_.Endpoints().MessageLoaded().data());

        assert_true(rc);

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
    PasswordPrompt reason,
    const identifier::Nym nymID,
    const std::size_t count) const noexcept -> void
{
    auto threads = api_.Storage().Internal().ThreadList(nymID, false);

    for (const auto& it : threads) {
        const auto threadID = api_.Factory().IdentifierFromBase58(it.first);
        thread_preload_thread(
            api_.Factory().PasswordPrompt(reason), nymID, threadID, 0, count);
    }
}

auto Activity::add_blockchain_transaction(
    const eLock& lock,
    const identifier::Nym& nym,
    const blockchain::block::Transaction& tx) const noexcept -> bool
{
    const auto& txid = tx.ID();
    // TODO allocator
    const auto incoming = tx.AssociatedRemoteContacts(api_, nym, {});
    LogTrace()()("transaction ")(txid.asHex())(" is associated with ")(
        incoming.size())(" contacts")
        .Flush();
    const auto existing =
        api_.Storage().Internal().BlockchainThreadMap(nym, txid);
    auto added = UnallocatedVector<identifier::Generic>{};
    auto removed = UnallocatedVector<identifier::Generic>{};
    std::ranges::set_difference(incoming, existing, std::back_inserter(added));
    std::ranges::set_difference(
        existing, incoming, std::back_inserter(removed));
    auto output{true};
    const auto chains = tx.Chains({});  // TODO allocator

    for (const auto& thread : added) {
        if (thread.empty()) { continue; }

        if (verify_thread_exists(nym, thread)) {
            auto saved{true};
            std::ranges::for_each(chains, [&](const auto& chain) {
                saved &= api_.Storage().Internal().Store(
                    nym, thread, chain, txid, tx.asBitcoin().Timestamp());
            });

            if (saved) { publish(nym, thread); }

            output &= saved;
        } else {
            output = false;
        }
    }

    for (const auto& thread : removed) {
        if (thread.empty()) { continue; }

        auto saved{true};
        const auto c = tx.Chains({});  // TODO allocator
        std::ranges::for_each(c, [&](const auto& chain) {
            saved &= api_.Storage().Internal().RemoveBlockchainThreadItem(
                nym, thread, chain, txid);
        });

        if (saved) { publish(nym, thread); }

        output &= saved;
    }

    if (0 == incoming.size()) {
        api_.Storage().Internal().UnaffiliatedBlockchainTransaction(nym, txid);
    }

    const auto proto = tx.Internal().asBitcoin().Serialize(api_);

    if (false == proto.has_value()) {
        LogError()()("failed to serialize transaction ")(txid.asHex()).Flush();

        return false;
    }

    std::ranges::for_each(chains, [&](const auto& chain) {
        get_blockchain(lock, nym).Send([&] {
            auto out = opentxs::network::zeromq::tagged_message(
                WorkType::BlockchainNewTransaction, true);
            out.AddFrame(txid);
            out.AddFrame(chain);
            out.Internal().AddFrame(proto.value());

            return out;
        }());
    });

    return output;
}

auto Activity::AddBlockchainTransaction(
    const api::crypto::Blockchain& crypto,
    const blockchain::block::Transaction& transaction) const noexcept -> bool
{
    auto lock = eLock(shared_lock_);

    // TODO allocator
    for (const auto& nym : transaction.AssociatedLocalNyms(crypto, {})) {
        LogTrace()()("blockchain transaction ")(transaction.ID().asHex())(
            " is relevant to local nym ")(nym, api_.Crypto())
            .Flush();

        assert_false(nym.empty());

        if (false == add_blockchain_transaction(lock, nym, transaction)) {
            return false;
        }
    }

    return true;
}

auto Activity::AddPaymentEvent(
    const identifier::Nym& nymID,
    const identifier::Generic& threadID,
    const otx::client::StorageBox type,
    const identifier::Generic& itemID,
    const identifier::Generic& workflowID,
    Time time) const noexcept -> bool
{
    auto lock = eLock(shared_lock_);

    if (false == verify_thread_exists(nymID, threadID)) { return false; }

    const bool saved = api_.Storage().Internal().Store(
        nymID, threadID, itemID, time, {}, {}, type, workflowID);

    if (saved) { publish(nymID, threadID); }

    return saved;
}

auto Activity::Cheque(
    const identifier::Nym& nym,
    const identifier::Generic& workflowID) const noexcept
    -> Activity::ChequeData
{
    auto output = ChequeData{
        nullptr, api_.Factory().Internal().Session().UnitDefinition()};
    auto& [cheque, contract] = output;
    auto [type, state] =
        api_.Storage().Internal().PaymentWorkflowState(nym, workflowID);
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
        case otx::client::PaymentWorkflowType::OutgoingCash:
        case otx::client::PaymentWorkflowType::IncomingCash:
        default: {
            LogError()()("Wrong workflow type.").Flush();

            return output;
        }
    }

    auto workflow = proto::PaymentWorkflow{};

    if (false == api_.Storage().Internal().Load(nym, workflowID, workflow)) {
        LogError()()("Workflow ")(workflowID, api_.Crypto())(" for nym ")(
            nym, api_.Crypto())(" can not be loaded.")
            .Flush();

        return output;
    }

    auto instantiated = session::Workflow::InstantiateCheque(api_, workflow);
    cheque = std::move(std::get<1>(instantiated));

    assert_false(nullptr == cheque);

    const auto& unit = cheque->GetInstrumentDefinitionID();

    try {
        contract = api_.Wallet().Internal().UnitDefinition(unit);
    } catch (...) {
        LogError()()("Unable to load unit definition contract.").Flush();
    }

    return output;
}

auto Activity::Transfer(
    const identifier::Nym& nym,
    [[maybe_unused]] const UnallocatedCString& id,
    const identifier::Generic& workflowID) const noexcept
    -> Activity::TransferData
{
    auto output = TransferData{
        nullptr, api_.Factory().Internal().Session().UnitDefinition()};
    auto& [transfer, contract] = output;
    auto [type, state] =
        api_.Storage().Internal().PaymentWorkflowState(nym, workflowID);

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
        case otx::client::PaymentWorkflowType::OutgoingCash:
        case otx::client::PaymentWorkflowType::IncomingCash:
        default: {
            LogError()()("Wrong workflow type").Flush();

            return output;
        }
    }

    auto workflow = proto::PaymentWorkflow{};

    if (false == api_.Storage().Internal().Load(nym, workflowID, workflow)) {
        LogError()()("Workflow ")(workflowID, api_.Crypto())(" for nym ")(
            nym, api_.Crypto())(" can not be loaded")
            .Flush();

        return output;
    }

    auto instantiated = session::Workflow::InstantiateTransfer(api_, workflow);
    transfer = std::move(std::get<1>(instantiated));

    assert_false(nullptr == transfer);

    if (0 == workflow.account_size()) {
        LogError()()("Workflow does not list any accounts.").Flush();

        return output;
    }

    const auto unit = api_.Storage().Internal().AccountContract(
        api_.Factory().AccountIDFromBase58(workflow.account(0)));

    if (unit.empty()) {
        LogError()()("Unable to calculate unit definition id.").Flush();

        return output;
    }

    try {
        contract = api_.Wallet().Internal().UnitDefinition(unit);
    } catch (...) {
        LogError()()("Unable to load unit definition contract.").Flush();
    }

    return output;
}

auto Activity::get_publisher(const identifier::Nym& nymID) const noexcept
    -> const opentxs::network::zeromq::socket::Publish&
{
    UnallocatedCString endpoint{};

    return get_publisher(nymID, endpoint);
}

auto Activity::get_blockchain(const eLock&, const identifier::Nym& nymID)
    const noexcept -> const opentxs::network::zeromq::socket::Publish&
{
    auto it = blockchain_publishers_.find(nymID);

    if (blockchain_publishers_.end() != it) { return it->second; }

    const auto endpoint = api_.Endpoints().BlockchainTransactions(nymID);
    const auto& [publisher, inserted] =
        blockchain_publishers_.emplace(nymID, start_publisher(endpoint.data()));

    assert_true(inserted);

    return publisher->second.get();
}

auto Activity::get_publisher(
    const identifier::Nym& nymID,
    UnallocatedCString& endpoint) const noexcept
    -> const opentxs::network::zeromq::socket::Publish&
{
    endpoint = api_.Endpoints().ThreadUpdate(nymID.asBase58(api_.Crypto()));
    const auto lock = Lock{publisher_lock_};
    auto it = thread_publishers_.find(nymID);

    if (thread_publishers_.end() != it) { return it->second; }

    const auto& [publisher, inserted] =
        thread_publishers_.emplace(nymID, start_publisher(endpoint));

    assert_true(inserted);

    return publisher->second.get();
}

auto Activity::Mail(
    const identifier::Nym& nym,
    const Message& mail,
    const otx::client::StorageBox box,
    const UnallocatedCString& text) const noexcept -> identifier::Generic
{
    auto id = identifier::Generic{};
    mail.CalculateContractID(id);
    const auto itemID = id.asBase58(api_.Crypto());
    mail_.CacheText(nym, id, box, text);
    const auto data = UnallocatedCString{String::Factory(mail)->Get()};
    const auto participantNymID = api_.Factory().NymIDFromBase58(
        [&]() -> auto& {
            const auto localName = String::Factory(nym, api_.Crypto());

            if (localName->Compare(mail.nym_id2_)) {
                // This is an incoming message. The contact id is the sender's
                // id.

                return mail.nym_id_;
            } else {
                // This is an outgoing message. The contact id is the
                // recipient's id.

                return mail.nym_id2_;
            }
        }()
                     ->Bytes());
    const auto contact = nym_to_contact(participantNymID);

    assert_false(nullptr == contact);

    auto lock = eLock(shared_lock_);
    const auto& alias = contact->Label();
    const auto& contactID = contact->ID();

    if (false == verify_thread_exists(nym, contactID)) { return {}; }

    const bool saved = api_.Storage().Internal().Store(
        nym,
        contactID,
        id,
        seconds_since_epoch(mail.time_).value(),
        alias,
        data,
        box,
        {});

    if (saved) {
        publish(nym, contactID);

        return id;
    }

    return {};
}

auto Activity::Mail(
    const identifier::Nym& nym,
    const otx::client::StorageBox box) const noexcept -> ObjectList
{
    return api_.Storage().Internal().NymBoxList(nym, box);
}

auto Activity::Mail(
    const identifier::Nym& nym,
    const Message& mail,
    const otx::client::StorageBox box,
    const PeerObject& text) const noexcept -> identifier::Generic
{
    return Mail(nym, mail, box, [&]() -> UnallocatedCString {
        if (const auto& out = text.Message(); out) {

            return *out;
        } else {

            return {};
        }
    }());
}

auto Activity::MailRemove(
    const identifier::Nym& nym,
    const identifier::Generic& id,
    const otx::client::StorageBox box) const noexcept -> bool
{
    return api_.Storage().Internal().RemoveNymBoxItem(nym, box, id);
}

auto Activity::MarkRead(
    const identifier::Nym& nymId,
    const identifier::Generic& threadId,
    const identifier::Generic& itemId) const noexcept -> bool
{
    return api_.Storage().Internal().SetReadState(
        nymId, threadId, itemId, false);
}

auto Activity::MarkUnread(
    const identifier::Nym& nymId,
    const identifier::Generic& threadId,
    const identifier::Generic& itemId) const noexcept -> bool
{
    return api_.Storage().Internal().SetReadState(
        nymId, threadId, itemId, true);
}

auto Activity::nym_to_contact(const identifier::Nym& nymID) const noexcept
    -> std::shared_ptr<const Contact>
{
    const auto contactID = contact_.NymToContact(nymID);

    return contact_.Contact(contactID);
}

auto Activity::PaymentText(
    const identifier::Nym& nym,
    const UnallocatedCString& id,
    const identifier::Generic& workflowID) const noexcept
    -> std::shared_ptr<const UnallocatedCString>
{
    std::shared_ptr<UnallocatedCString> output;
    auto [type, state] =
        api_.Storage().Internal().PaymentWorkflowState(nym, workflowID);
    [[maybe_unused]] const auto& notUsed = state;

    switch (type) {
        case otx::client::PaymentWorkflowType::OutgoingCheque: {
            output.reset(new UnallocatedCString("Sent cheque"));
        } break;
        case otx::client::PaymentWorkflowType::IncomingCheque: {
            output.reset(new UnallocatedCString("Received cheque"));
        } break;
        case otx::client::PaymentWorkflowType::OutgoingTransfer: {
            output.reset(new UnallocatedCString("Sent transfer"));
        } break;
        case otx::client::PaymentWorkflowType::IncomingTransfer: {
            output.reset(new UnallocatedCString("Received transfer"));
        } break;
        case otx::client::PaymentWorkflowType::InternalTransfer: {
            output.reset(new UnallocatedCString("Internal transfer"));
        } break;

        case otx::client::PaymentWorkflowType::Error:
        case otx::client::PaymentWorkflowType::OutgoingInvoice:
        case otx::client::PaymentWorkflowType::IncomingInvoice:
        case otx::client::PaymentWorkflowType::OutgoingCash:
        case otx::client::PaymentWorkflowType::IncomingCash:
        default: {

            return output;
        }
    }

    auto workflow = proto::PaymentWorkflow{};

    if (false == api_.Storage().Internal().Load(nym, workflowID, workflow)) {
        LogError()()("Workflow ")(workflowID, api_.Crypto())(" for nym ")(
            nym, api_.Crypto())(" can not be loaded.")
            .Flush();

        return output;
    }

    switch (type) {
        case otx::client::PaymentWorkflowType::OutgoingCheque:
        case otx::client::PaymentWorkflowType::IncomingCheque:
        case otx::client::PaymentWorkflowType::OutgoingInvoice:
        case otx::client::PaymentWorkflowType::IncomingInvoice: {
            auto chequeData = Cheque(nym, workflowID);
            const auto& [cheque, contract] = chequeData;

            assert_false(nullptr == cheque);

            if (0 < contract->Version()) {
                const auto& definition =
                    opentxs::display::GetDefinition(contract->UnitOfAccount());
                const auto amount = definition.Format(cheque->GetAmount());

                if (0 < amount.size()) {
                    const UnallocatedCString text =
                        *output + UnallocatedCString{" for "} + amount;
                    *output = text;
                }
            }
        } break;

        case otx::client::PaymentWorkflowType::OutgoingTransfer:
        case otx::client::PaymentWorkflowType::IncomingTransfer:
        case otx::client::PaymentWorkflowType::InternalTransfer: {
            auto transferData = Transfer(nym, id, workflowID);
            const auto& [transfer, contract] = transferData;

            assert_false(nullptr == transfer);

            if (0 < contract->Version()) {
                const auto& definition =
                    display::GetDefinition(contract->UnitOfAccount());
                const auto amount = definition.Format(transfer->GetAmount());

                if (0 < amount.size()) {
                    const UnallocatedCString text =
                        *output + UnallocatedCString{" for "} + amount;
                    *output = text;
                }
            }
        } break;
        case otx::client::PaymentWorkflowType::Error:
        case otx::client::PaymentWorkflowType::OutgoingCash:
        case otx::client::PaymentWorkflowType::IncomingCash:
        default: {

            return nullptr;
        }
    }

    return output;
}

auto Activity::PreloadActivity(
    const identifier::Nym& nymID,
    const std::size_t count,
    const PasswordPrompt& reason) const noexcept -> void
{
    std::thread preload(
        &Activity::activity_preload_thread,
        this,
        api_.Factory().PasswordPrompt(reason),
        nymID,
        count);
    preload.detach();
}

auto Activity::PreloadThread(
    const identifier::Nym& nymID,
    const identifier::Generic& threadID,
    const std::size_t start,
    const std::size_t count,
    const PasswordPrompt& reason) const noexcept -> void
{
    std::thread preload(
        &Activity::thread_preload_thread,
        this,
        api_.Factory().PasswordPrompt(reason),
        nymID,
        threadID,
        start,
        count);
    preload.detach();
}

auto Activity::publish(
    const identifier::Nym& nymID,
    const identifier::Generic& threadID) const noexcept -> void
{
    const auto& socket = get_publisher(nymID);
    socket.Send([&] {
        auto work = opentxs::network::zeromq::tagged_message(
            WorkType::ContactActivityUpdated, true);
        work.AddFrame(threadID);

        return work;
    }());
}

auto Activity::start_publisher(
    const UnallocatedCString& endpoint) const noexcept -> OTZMQPublishSocket
{
    auto output = api_.Network().ZeroMQ().Context().Internal().PublishSocket();
    const auto started = output->Start(endpoint);

    assert_true(started);

    LogDetail()()("Publisher started on ")(endpoint).Flush();

    return output;
}

auto Activity::Thread(
    const identifier::Nym& nymID,
    const identifier::Generic& threadID,
    proto::StorageThread& output) const noexcept -> bool
{
    auto lock = sLock(shared_lock_);

    if (false == api_.Storage().Internal().Load(nymID, threadID, output)) {
        return false;
    }

    return true;
}

auto Activity::Thread(
    const identifier::Nym& nymID,
    const identifier::Generic& threadID,
    Writer&& output) const noexcept -> bool
{
    auto lock = sLock(shared_lock_);

    auto serialized = proto::StorageThread{};
    if (false == api_.Storage().Internal().Load(nymID, threadID, serialized)) {
        return false;
    }

    return write(serialized, std::move(output));
}

auto Activity::thread_preload_thread(
    PasswordPrompt reason,
    const identifier::Nym nymID,
    const identifier::Generic threadID,
    const std::size_t start,
    const std::size_t count) const noexcept -> void
{
    auto thread = proto::StorageThread{};

    if (false == api_.Storage().Internal().Load(nymID, threadID, thread)) {
        LogError()()("Unable to load thread ")(threadID, api_.Crypto())(
            " for nym ")(nymID, api_.Crypto())
            .Flush();

        return;
    }

    const auto size = std::size_t(thread.item_size());
    auto cached = 0_uz;

    if (start > size) {
        LogError()()("Error: start larger than size (")(start)(" / ")(size)(")")
            .Flush();

        return;
    }

    assert_true((size - start) <= std::numeric_limits<int>::max());

    for (auto i = (size - start); i > 0_uz; --i) {
        if (cached >= count) { break; }

        const auto& item = thread.item(static_cast<int>(i - 1_uz));
        const auto& box = static_cast<otx::client::StorageBox>(item.box());

        switch (box) {
            case otx::client::StorageBox::MAILINBOX:
            case otx::client::StorageBox::MAILOUTBOX: {
                LogTrace()()("Preloading item ")(item.id())(" in thread ")(
                    threadID, api_.Crypto())
                    .Flush();
                mail_.GetText(
                    nymID,
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
    -> UnallocatedCString
{
    UnallocatedCString endpoint{};
    get_publisher(nym, endpoint);

    return endpoint;
}

auto Activity::Threads(const identifier::Nym& nymID, const bool unreadOnly)
    const noexcept -> ObjectList
{
    auto output = api_.Storage().Internal().ThreadList(nymID, unreadOnly);

    for (auto& it : output) {
        const auto threadID = api_.Factory().IdentifierFromBase58(it.first);
        auto& label = it.second;

        if (label.empty()) {
            auto contact = contact_.Contact(threadID);

            if (contact) {
                const auto& name = contact->Label();

                if (label != name) {
                    api_.Storage().Internal().SetThreadAlias(
                        nymID, threadID, name);
                    label = name;
                }
            }
        }
    }

    return output;
}

auto Activity::UnreadCount(const identifier::Nym& nym) const noexcept
    -> std::size_t
{
    auto output = 0_uz;

    const auto& threads = api_.Storage().Internal().ThreadList(nym, true);

    for (const auto& it : threads) {
        const auto& threadId = api_.Factory().IdentifierFromBase58(it.first);
        output += api_.Storage().Internal().UnreadCount(nym, threadId);
    }

    return output;
}

auto Activity::verify_thread_exists(
    const identifier::Nym& nym,
    const identifier::Generic& thread) const noexcept -> bool
{
    const auto list = api_.Storage().Internal().ThreadList(nym, false);

    for (const auto& it : list) {
        const auto id = api_.Factory().IdentifierFromBase58(it.first);

        if (id == thread) { return true; }
    }

    return api_.Storage().Internal().CreateThread(nym, thread, {thread});
}

Activity::~Activity() = default;
}  // namespace opentxs::api::session::imp
