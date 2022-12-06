// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::UnitType
// IWYU pragma: no_forward_declare opentxs::api::session::OTX
// IWYU pragma: no_forward_declare opentxs::contract::peer::ConnectionInfoType
// IWYU pragma: no_forward_declare opentxs::contract::peer::SecretType
// IWYU pragma: no_forward_declare opentxs::identity::wot::claim::ClaimType
// IWYU pragma: no_forward_declare opentxs::identity::wot::claim::SectionType
// IWYU pragma: no_forward_declare opentxs::otx::LastReplyStatus
// IWYU pragma: no_forward_declare opentxs::otx::client::PaymentWorkflowState

#include "api/session/OTX.hpp"  // IWYU pragma: associated

#include <PeerRequest.pb.h>
#include <ServerContract.pb.h>
#include <ServerReply.pb.h>
#include <atomic>
#include <chrono>
#include <compare>
#include <ctime>
#include <memory>
#include <ratio>
#include <stdexcept>
#include <string_view>
#include <tuple>

#include "internal/api/FactoryAPI.hpp"
#include "internal/api/Settings.hpp"
#include "internal/api/session/Client.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/api/session/Factory.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/core/Factory.hpp"
#include "internal/core/String.hpp"
#include "internal/core/contract/ServerContract.hpp"
#include "internal/core/contract/peer/BailmentNotice.hpp"
#include "internal/core/contract/peer/BailmentReply.hpp"
#include "internal/core/contract/peer/BailmentRequest.hpp"
#include "internal/core/contract/peer/ConnectionReply.hpp"
#include "internal/core/contract/peer/ConnectionRequest.hpp"
#include "internal/core/contract/peer/NoticeAcknowledgement.hpp"
#include "internal/core/contract/peer/OutBailmentReply.hpp"
#include "internal/core/contract/peer/OutBailmentRequest.hpp"
#include "internal/core/contract/peer/StoreSecret.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/socket/Publish.hpp"
#include "internal/network/zeromq/socket/Pull.hpp"
#include "internal/network/zeromq/socket/Subscribe.hpp"
#include "internal/network/zeromq/socket/Types.hpp"
#include "internal/otx/client/OTPayment.hpp"
#include "internal/otx/client/obsolete/OT_API.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/otx/common/Cheque.hpp"
#include "internal/otx/consensus/Server.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/util/Editor.hpp"
#include "internal/util/Flag.hpp"
#include "internal/util/Lockable.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Pimpl.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/OTX.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/api/session/Workflow.hpp"
#include "opentxs/core/Contact.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/identity/wot/claim/Data.hpp"
#include "opentxs/identity/wot/claim/Group.hpp"  // IWYU pragma: keep
#include "opentxs/identity/wot/claim/Item.hpp"
#include "opentxs/identity/wot/claim/SectionType.hpp"  // IWYU pragma: keep
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/otx/Reply.hpp"
#include "opentxs/otx/ServerReplyType.hpp"  // IWYU pragma: keep
#include "opentxs/otx/Types.hpp"
#include "opentxs/otx/client/PaymentWorkflowState.hpp"  // IWYU pragma: keep
#include "opentxs/otx/client/PaymentWorkflowType.hpp"   // IWYU pragma: keep
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/NymEditor.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WorkType.hpp"
#include "otx/client/PaymentTasks.hpp"
#include "otx/client/StateMachine.hpp"

#define VALIDATE_NYM(a)                                                        \
    if (a.empty()) {                                                           \
        LogError()(OT_PRETTY_CLASS())("Invalid ")(#a)(".").Flush();            \
                                                                               \
        return error_task();                                                   \
    }                                                                          \
    static_assert(0 < sizeof(char))  // NOTE silence -Wextra-semi-stmt

#define CHECK_SERVER(a, b)                                                     \
    VALIDATE_NYM(a);                                                           \
                                                                               \
    if (b.empty()) {                                                           \
        LogError()(OT_PRETTY_CLASS())("Invalid ")(#b)(".").Flush();            \
                                                                               \
        return error_task();                                                   \
    }                                                                          \
    static_assert(0 < sizeof(char))  // NOTE silence -Wextra-semi-stmt

#define CHECK_ARGS(a, b, c)                                                    \
    CHECK_SERVER(a, b);                                                        \
                                                                               \
    if (c.empty()) {                                                           \
        LogError()(OT_PRETTY_CLASS())("Invalid ")(#c)(".").Flush();            \
                                                                               \
        return error_task();                                                   \
    }                                                                          \
    static_assert(0 < sizeof(char))  // NOTE silence -Wextra-semi-stmt

#define YIELD_OTX(a)                                                           \
    if (!running_) { return false; }                                           \
                                                                               \
    Sleep(std::chrono::milliseconds(a))

#define SHUTDOWN_OTX() YIELD_OTX(50)

namespace
{
constexpr auto CONTACT_REFRESH_DAYS = 1;
constexpr auto INTRODUCTION_SERVER_KEY = "introduction_server_id";
constexpr auto MASTER_SECTION = "Master";
}  // namespace

namespace zmq = opentxs::network::zeromq;

namespace opentxs::factory
{
auto OTX(
    const Flag& running,
    const api::session::Client& client,
    ContextLockCallback lockCallback) noexcept
    -> std::unique_ptr<api::session::OTX>
{
    using ReturnType = api::session::imp::OTX;

    return std::make_unique<ReturnType>(
        running, client, std::move(lockCallback));
}
}  // namespace opentxs::factory

namespace opentxs::api::session::imp
{
OTX::OTX(
    const Flag& running,
    const api::session::Client& client,
    ContextLockCallback lockCallback)
    : lock_callback_(std::move(lockCallback))
    , running_(running)
    , api_(client)
    , introduction_server_lock_()
    , nym_fetch_lock_()
    , task_status_lock_()
    , refresh_counter_(0)
    , operations_()
    , server_nym_fetch_()
    , missing_nyms_()
    , outdated_nyms_()
    , missing_servers_()
    , missing_unit_definitions_()
    , introduction_server_id_()
    , task_status_()
    , task_message_id_()
    , account_subscriber_callback_(zmq::ListenCallback::Factory(
          [this](const zmq::Message& message) -> void {
              this->process_account(message);
          }))
    , account_subscriber_([&] {
        const auto endpoint = api_.Endpoints().AccountUpdate();
        LogDetail()(OT_PRETTY_CLASS())("Connecting to ")(endpoint.data())
            .Flush();
        auto out = api_.Network().ZeroMQ().Internal().SubscribeSocket(
            account_subscriber_callback_.get(), "OTX account");
        const auto start = out->Start(endpoint.data());

        OT_ASSERT(start);

        return out;
    }())
    , notification_listener_callback_(zmq::ListenCallback::Factory(
          [this](const zmq::Message& message) -> void {
              this->process_notification(message);
          }))
    , notification_listener_([&] {
        auto out = api_.Network().ZeroMQ().Internal().PullSocket(
            notification_listener_callback_,
            zmq::socket::Direction::Bind,
            "OTX notification listener");
        const auto start = out->Start(
            api_.Endpoints().Internal().ProcessPushNotification().data());

        OT_ASSERT(start);

        return out;
    }())
    , find_nym_callback_(zmq::ListenCallback::Factory(
          [this](const zmq::Message& message) -> void {
              this->find_nym(message);
          }))
    , find_nym_listener_([&] {
        auto out = api_.Network().ZeroMQ().Internal().PullSocket(
            find_nym_callback_,
            zmq::socket::Direction::Bind,
            "OTX nym listener");
        const auto start = out->Start(api_.Endpoints().FindNym().data());

        OT_ASSERT(start);

        return out;
    }())
    , find_server_callback_(zmq::ListenCallback::Factory(
          [this](const zmq::Message& message) -> void {
              this->find_server(message);
          }))
    , find_server_listener_([&] {
        auto out = api_.Network().ZeroMQ().Internal().PullSocket(
            find_server_callback_,
            zmq::socket::Direction::Bind,
            "OTX server listener");
        const auto start = out->Start(api_.Endpoints().FindServer().data());

        OT_ASSERT(start);

        return out;
    }())
    , find_unit_callback_(zmq::ListenCallback::Factory(
          [this](const zmq::Message& message) -> void {
              this->find_unit(message);
          }))
    , find_unit_listener_([&] {
        auto out = api_.Network().ZeroMQ().Internal().PullSocket(
            find_unit_callback_,
            zmq::socket::Direction::Bind,
            "OTX unit listener");
        const auto start =
            out->Start(api_.Endpoints().FindUnitDefinition().data());

        OT_ASSERT(start);

        return out;
    }())
    , task_finished_([&] {
        auto out = api_.Network().ZeroMQ().Internal().PublishSocket();
        const auto start = out->Start(api_.Endpoints().TaskComplete().data());

        OT_ASSERT(start);

        return out;
    }())
    , messagability_([&] {
        auto out = api_.Network().ZeroMQ().Internal().PublishSocket();
        const auto start = out->Start(api_.Endpoints().Messagability().data());

        OT_ASSERT(start);

        return out;
    }())
    , auto_process_inbox_(Flag::Factory(true))
    , next_task_id_(0)
    , shutdown_(false)
    , shutdown_lock_()
    , reason_(api_.Factory().PasswordPrompt("Refresh OTX data with notary"))
{
    // WARNING: do not access api_.Wallet() during construction
}

auto OTX::AcknowledgeBailment(
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID,
    const identifier::Nym& targetNymID,
    const identifier::Generic& requestID,
    const UnallocatedCString& instructions,
    const otx::client::SetID setID) const -> OTX::BackgroundTask
{
    CHECK_ARGS(localNymID, serverID, targetNymID);
    VALIDATE_NYM(requestID);

    start_introduction_server(localNymID);
    const auto nym = api_.Wallet().Nym(localNymID);
    auto time = std::time_t{0};
    auto serializedRequest = proto::PeerRequest{};
    if (false == api_.Wallet().Internal().PeerRequest(
                     nym->ID(),
                     requestID,
                     otx::client::StorageBox::INCOMINGPEERREQUEST,
                     time,
                     serializedRequest)) {
        LogError()(OT_PRETTY_CLASS())("Failed to load request.").Flush();

        return error_task();
    }

    auto recipientNym = api_.Wallet().Nym(targetNymID);

    try {
        auto instantiatedRequest =
            api_.Factory().InternalSession().BailmentRequest(
                recipientNym, serializedRequest);
        auto peerreply = api_.Factory().InternalSession().BailmentReply(
            nym,
            instantiatedRequest->Initiator(),
            requestID,
            serverID,
            instructions,
            reason_);

        if (setID) { setID(peerreply->ID()); }

        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::PeerReplyTask>(
            {targetNymID,
             peerreply.as<contract::peer::Reply>(),
             instantiatedRequest.as<contract::peer::Request>()});
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return error_task();
    }
}

auto OTX::AcknowledgeConnection(
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID,
    const identifier::Nym& recipientID,
    const identifier::Generic& requestID,
    const bool ack,
    const UnallocatedCString& url,
    const UnallocatedCString& login,
    const UnallocatedCString& password,
    const UnallocatedCString& key,
    const otx::client::SetID setID) const -> OTX::BackgroundTask
{
    CHECK_ARGS(localNymID, serverID, recipientID);
    VALIDATE_NYM(requestID);

    start_introduction_server(localNymID);
    const auto nym = api_.Wallet().Nym(localNymID);

    if (ack) {
        if (url.empty()) {
            LogError()(OT_PRETTY_CLASS())("Warning: url is empty.").Flush();
        }

        if (login.empty()) {
            LogError()(OT_PRETTY_CLASS())("Warning: login is empty.").Flush();
        }

        if (password.empty()) {
            LogError()(OT_PRETTY_CLASS())("Warning: password is empty.")
                .Flush();
        }

        if (key.empty()) {
            LogError()(OT_PRETTY_CLASS())("Warning: key is empty.").Flush();
        }
    }

    auto time = std::time_t{0};
    auto serializedRequest = proto::PeerRequest{};
    if (false == api_.Wallet().Internal().PeerRequest(
                     nym->ID(),
                     requestID,
                     otx::client::StorageBox::INCOMINGPEERREQUEST,
                     time,
                     serializedRequest)) {
        LogError()(OT_PRETTY_CLASS())("Failed to load request.").Flush();

        return error_task();
    }

    auto recipientNym = api_.Wallet().Nym(recipientID);

    try {
        auto instantiatedRequest =
            api_.Factory().InternalSession().BailmentRequest(
                recipientNym, serializedRequest);
        auto peerreply = api_.Factory().InternalSession().ConnectionReply(
            nym,
            instantiatedRequest->Initiator(),
            requestID,
            serverID,
            ack,
            url,
            login,
            password,
            key,
            reason_);

        if (setID) { setID(peerreply->ID()); }

        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::PeerReplyTask>(
            {recipientID,
             peerreply.as<contract::peer::Reply>(),
             instantiatedRequest.as<contract::peer::Request>()});
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return error_task();
    }
}

auto OTX::AcknowledgeNotice(
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID,
    const identifier::Nym& recipientID,
    const identifier::Generic& requestID,
    const bool ack,
    const otx::client::SetID setID) const -> OTX::BackgroundTask
{
    CHECK_ARGS(localNymID, serverID, recipientID);
    VALIDATE_NYM(requestID);

    start_introduction_server(localNymID);
    const auto nym = api_.Wallet().Nym(localNymID);
    std::time_t time{0};
    auto serializedRequest = proto::PeerRequest{};
    if (false == api_.Wallet().Internal().PeerRequest(
                     nym->ID(),
                     requestID,
                     otx::client::StorageBox::INCOMINGPEERREQUEST,
                     time,
                     serializedRequest)) {
        LogError()(OT_PRETTY_CLASS())("Failed to load request.").Flush();

        return error_task();
    }

    auto recipientNym = api_.Wallet().Nym(recipientID);

    try {
        auto instantiatedRequest =
            api_.Factory().InternalSession().BailmentRequest(
                recipientNym, serializedRequest);
        auto peerreply = api_.Factory().InternalSession().ReplyAcknowledgement(
            nym,
            instantiatedRequest->Initiator(),
            requestID,
            serverID,
            instantiatedRequest->Type(),
            ack,
            reason_);

        if (setID) { setID(peerreply->ID()); }

        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::PeerReplyTask>(
            {recipientID,
             peerreply.as<contract::peer::Reply>(),
             instantiatedRequest.as<contract::peer::Request>()});
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return error_task();
    }
}

auto OTX::AcknowledgeOutbailment(
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID,
    const identifier::Nym& recipientID,
    const identifier::Generic& requestID,
    const UnallocatedCString& details,
    const otx::client::SetID setID) const -> OTX::BackgroundTask
{
    CHECK_ARGS(localNymID, serverID, recipientID);
    VALIDATE_NYM(requestID);

    start_introduction_server(localNymID);
    const auto nym = api_.Wallet().Nym(localNymID);
    std::time_t time{0};
    auto serializedRequest = proto::PeerRequest{};
    if (false == api_.Wallet().Internal().PeerRequest(
                     nym->ID(),
                     requestID,
                     otx::client::StorageBox::INCOMINGPEERREQUEST,
                     time,
                     serializedRequest)) {
        LogError()(OT_PRETTY_CLASS())("Failed to load request.").Flush();

        return error_task();
    }

    auto recipientNym = api_.Wallet().Nym(recipientID);

    try {
        auto instantiatedRequest =
            api_.Factory().InternalSession().BailmentRequest(
                recipientNym, serializedRequest);
        auto peerreply = api_.Factory().InternalSession().OutbailmentReply(
            nym,
            instantiatedRequest->Initiator(),
            requestID,
            serverID,
            details,
            reason_);

        if (setID) { setID(peerreply->ID()); }

        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::PeerReplyTask>(
            {recipientID,
             peerreply.as<contract::peer::Reply>(),
             instantiatedRequest.as<contract::peer::Request>()});
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return error_task();
    }
}

auto OTX::add_task(const TaskID taskID, const otx::client::ThreadStatus status)
    const -> OTX::BackgroundTask
{
    Lock lock(task_status_lock_);

    if (0 != task_status_.count(taskID)) { return error_task(); }

    std::promise<Result> promise{};
    [[maybe_unused]] auto [it, added] = task_status_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(taskID),
        std::forward_as_tuple(status, std::move(promise)));

    OT_ASSERT(added);

    return {it->first, it->second.second.get_future()};
}

void OTX::associate_message_id(
    const identifier::Generic& messageID,
    const TaskID taskID) const
{
    Lock lock(task_status_lock_);
    task_message_id_.emplace(taskID, messageID);
}

auto OTX::can_deposit(
    const OTPayment& payment,
    const identifier::Nym& recipient,
    const identifier::Generic& accountIDHint,
    identifier::Notary& depositServer,
    identifier::UnitDefinition& unitID,
    identifier::Generic& depositAccount) const -> otx::client::Depositability
{
    auto nymID = identifier::Nym{};

    if (false == extract_payment_data(payment, nymID, depositServer, unitID)) {

        return otx::client::Depositability::INVALID_INSTRUMENT;
    }

    auto output = valid_recipient(payment, nymID, recipient);

    if (otx::client::Depositability::WRONG_RECIPIENT == output) {
        return output;
    }

    const bool registered =
        api_.InternalClient().OTAPI().IsNym_RegisteredAtServer(
            recipient, depositServer);

    if (false == registered) {
        schedule_download_nymbox(recipient, depositServer);
        LogDetail()(OT_PRETTY_CLASS())("Recipient nym ")(
            recipient)(" not registered on "
                       "server ")(depositServer)(".")
            .Flush();

        return otx::client::Depositability::NOT_REGISTERED;
    }

    output = valid_account(
        payment,
        recipient,
        depositServer,
        unitID,
        accountIDHint,
        depositAccount);

    switch (output) {
        case otx::client::Depositability::ACCOUNT_NOT_SPECIFIED: {
            LogError()(OT_PRETTY_CLASS())(
                ": Multiple valid accounts exist. "
                "This payment can not be automatically deposited.")
                .Flush();
        } break;
        case otx::client::Depositability::WRONG_ACCOUNT: {
            LogDetail()(OT_PRETTY_CLASS())(
                ": The specified account is not valid for this payment.")
                .Flush();
        } break;
        case otx::client::Depositability::NO_ACCOUNT: {

            LogDetail()(OT_PRETTY_CLASS())("Recipient ")(
                recipient)(" needs an account "
                           "for ")(unitID)(" on "
                                           "serve"
                                           "r"
                                           " ")(depositServer)(".")
                .Flush();
        } break;
        case otx::client::Depositability::READY: {
            LogDetail()(OT_PRETTY_CLASS())("Payment can be deposited.").Flush();
        } break;
        case otx::client::Depositability::WRONG_RECIPIENT:
        case otx::client::Depositability::INVALID_INSTRUMENT:
        case otx::client::Depositability::NOT_REGISTERED:
        case otx::client::Depositability::UNKNOWN:
        default: {
            OT_FAIL;
        }
    }

    return output;
}

auto OTX::can_message(
    const identifier::Nym& senderNymID,
    const identifier::Generic& recipientContactID,
    identifier::Nym& recipientNymID,
    identifier::Notary& serverID) const -> otx::client::Messagability
{
    const auto publish = [&](auto value) {
        return publish_messagability(senderNymID, recipientContactID, value);
    };
    auto senderNym = api_.Wallet().Nym(senderNymID);

    if (false == bool(senderNym)) {
        LogDetail()(OT_PRETTY_CLASS())("Unable to load sender nym ")(
            senderNymID)
            .Flush();

        return publish(otx::client::Messagability::MISSING_SENDER);
    }

    const bool canSign =
        senderNym->HasCapability(identity::NymCapability::SIGN_MESSAGE);

    if (false == canSign) {
        LogDetail()(OT_PRETTY_CLASS())("Sender nym ")(
            senderNymID)(" can not sign messages (no private "
                         "key).")
            .Flush();

        return publish(otx::client::Messagability::INVALID_SENDER);
    }

    const auto contact = api_.Contacts().Contact(recipientContactID);

    if (false == bool(contact)) {
        LogDetail()(OT_PRETTY_CLASS())("Recipient contact ")(
            recipientContactID)(" does not exist.")
            .Flush();

        return publish(otx::client::Messagability::MISSING_CONTACT);
    }

    const auto nyms = contact->Nyms();

    if (0 == nyms.size()) {
        LogDetail()(OT_PRETTY_CLASS())("Recipient contact ")(
            recipientContactID)(" does not have a nym.")
            .Flush();

        return publish(otx::client::Messagability::CONTACT_LACKS_NYM);
    }

    Nym_p recipientNym{nullptr};

    for (const auto& it : nyms) {
        recipientNym = api_.Wallet().Nym(it);

        if (recipientNym) {
            recipientNymID.Assign(it);
            break;
        }
    }

    if (false == bool(recipientNym)) {
        for (const auto& nymID : nyms) {
            outdated_nyms_.Push(next_task_id(), nymID);
        }

        LogDetail()(OT_PRETTY_CLASS())("Recipient contact ")(
            recipientContactID)(" credentials not "
                                "available.")
            .Flush();

        return publish(otx::client::Messagability::MISSING_RECIPIENT);
    }

    OT_ASSERT(recipientNym);

    const auto& claims = recipientNym->Claims();
    serverID.Assign(claims.PreferredOTServer());

    // TODO maybe some of the other nyms in this contact do specify a server
    if (serverID.empty()) {
        LogDetail()(OT_PRETTY_CLASS())("Recipient contact ")(
            recipientContactID)(", "
                                "nym ")(
            recipientNymID)(": credentials do not specify a server.")
            .Flush();
        outdated_nyms_.Push(next_task_id(), recipientNymID);

        return publish(otx::client::Messagability::NO_SERVER_CLAIM);
    }

    const bool registered =
        api_.InternalClient().OTAPI().IsNym_RegisteredAtServer(
            senderNymID, serverID);

    if (false == registered) {
        schedule_download_nymbox(senderNymID, serverID);
        LogDetail()(OT_PRETTY_CLASS())("Sender nym ")(
            senderNymID)(" not registered on "
                         "server ")(serverID)
            .Flush();

        return publish(otx::client::Messagability::UNREGISTERED);
    }

    return publish(otx::client::Messagability::READY);
}

auto OTX::CanDeposit(
    const identifier::Nym& recipientNymID,
    const OTPayment& payment) const -> otx::client::Depositability
{
    auto accountHint = identifier::Generic{};

    return CanDeposit(recipientNymID, accountHint, payment);
}

auto OTX::CanDeposit(
    const identifier::Nym& recipientNymID,
    const identifier::Generic& accountIDHint,
    const OTPayment& payment) const -> otx::client::Depositability
{
    auto serverID = identifier::Notary{};
    auto unitID = identifier::UnitDefinition{};
    auto accountID = identifier::Generic{};

    return can_deposit(
        payment, recipientNymID, accountIDHint, serverID, unitID, accountID);
}

auto OTX::CanMessage(
    const identifier::Nym& sender,
    const identifier::Generic& contact,
    const bool startIntroductionServer) const -> otx::client::Messagability
{
    const auto publish = [&](auto value) {
        return publish_messagability(sender, contact, value);
    };

    if (startIntroductionServer) { start_introduction_server(sender); }

    if (sender.empty()) {
        LogError()(OT_PRETTY_CLASS())("Invalid sender.").Flush();

        return publish(otx::client::Messagability::INVALID_SENDER);
    }

    if (contact.empty()) {
        LogError()(OT_PRETTY_CLASS())("Invalid recipient.").Flush();

        return publish(otx::client::Messagability::MISSING_CONTACT);
    }

    auto nymID = identifier::Nym{};
    auto serverID = identifier::Notary{};

    return can_message(sender, contact, nymID, serverID);
}

auto OTX::CheckTransactionNumbers(
    const identifier::Nym& nym,
    const identifier::Notary& serverID,
    const std::size_t quantity) const -> bool
{
    auto context = api_.Wallet().Internal().ServerContext(nym, serverID);

    if (false == bool(context)) {
        LogError()(OT_PRETTY_CLASS())("Nym is not registered").Flush();

        return false;
    }

    const auto available = context->AvailableNumbers();

    if (quantity <= available) { return true; }

    LogVerbose()(OT_PRETTY_CLASS())("Asking server for more numbers.").Flush();

    try {
        auto& queue = get_operations({nym, serverID});
        const auto output =
            queue.StartTask<otx::client::GetTransactionNumbersTask>({});
        const auto& taskID = output.first;

        if (0 == taskID) { return false; }

        auto status = Status(taskID);

        while (otx::client::ThreadStatus::RUNNING == status) {
            Sleep(100ms);
            status = Status(taskID);
        }

        if (otx::client::ThreadStatus::FINISHED_SUCCESS == status) {
            return true;
        }

        return false;
    } catch (...) {

        return false;
    }
}

auto OTX::ContextIdle(
    const identifier::Nym& nym,
    const identifier::Notary& server) const -> OTX::Finished
{
    try {
        auto& queue = get_operations({nym, server});

        return queue.Wait();
    } catch (...) {
        std::promise<void> empty{};
        auto output = empty.get_future();
        empty.set_value();

        return output;
    }
}

auto OTX::DepositCheques(const identifier::Nym& nymID) const -> std::size_t
{
    std::size_t output{0};
    const auto workflows = api_.Workflow().List(
        nymID,
        otx::client::PaymentWorkflowType::IncomingCheque,
        otx::client::PaymentWorkflowState::Conveyed);

    for (const auto& id : workflows) {
        const auto chequeState =
            api_.Workflow().LoadChequeByWorkflow(nymID, id);
        const auto& [state, cheque] = chequeState;

        if (otx::client::PaymentWorkflowState::Conveyed != state) { continue; }

        OT_ASSERT(cheque);

        if (queue_cheque_deposit(nymID, *cheque)) { ++output; }
    }

    return output;
}

auto OTX::DepositCheques(
    const identifier::Nym& nymID,
    const UnallocatedSet<identifier::Generic>& chequeIDs) const -> std::size_t
{
    auto output = 0_uz;

    if (chequeIDs.empty()) { return DepositCheques(nymID); }

    for (const auto& id : chequeIDs) {
        const auto chequeState = api_.Workflow().LoadCheque(nymID, id);
        const auto& [state, cheque] = chequeState;

        if (otx::client::PaymentWorkflowState::Conveyed != state) { continue; }

        OT_ASSERT(cheque);

        if (queue_cheque_deposit(nymID, *cheque)) { ++output; }
    }

    return output;
}

auto OTX::DepositPayment(
    const identifier::Nym& recipientNymID,
    const std::shared_ptr<const OTPayment>& payment) const
    -> OTX::BackgroundTask
{
    auto notUsed = identifier::Generic{};

    return DepositPayment(recipientNymID, notUsed, payment);
}

auto OTX::DepositPayment(
    const identifier::Nym& recipientNymID,
    const identifier::Generic& accountIDHint,
    const std::shared_ptr<const OTPayment>& payment) const
    -> OTX::BackgroundTask
{
    OT_ASSERT(payment);

    if (recipientNymID.empty()) {
        LogError()(OT_PRETTY_CLASS())("Invalid recipient.").Flush();

        return error_task();
    }

    auto serverID = identifier::Notary{};
    auto unitID = identifier::UnitDefinition{};
    auto accountID = identifier::Generic{};
    const auto status = can_deposit(
        *payment, recipientNymID, accountIDHint, serverID, unitID, accountID);

    try {
        switch (status) {
            case otx::client::Depositability::READY:
            case otx::client::Depositability::NOT_REGISTERED:
            case otx::client::Depositability::NO_ACCOUNT: {
                start_introduction_server(recipientNymID);
                auto& queue = get_operations({recipientNymID, serverID});

                return queue.payment_tasks_.Queue({unitID, accountID, payment});
            }
            default: {
                LogError()(OT_PRETTY_CLASS())(
                    ": Unable to queue payment for download (")(
                    static_cast<std::int8_t>(status))(")")
                    .Flush();

                return error_task();
            }
        }
    } catch (...) {
        return error_task();
    }
}

void OTX::DisableAutoaccept() const { auto_process_inbox_->Off(); }

auto OTX::DownloadMint(
    const identifier::Nym& nym,
    const identifier::Notary& server,
    const identifier::UnitDefinition& unit) const -> OTX::BackgroundTask
{
    CHECK_ARGS(nym, server, unit);

    try {
        start_introduction_server(nym);
        auto& queue = get_operations({nym, server});

        return queue.StartTask<otx::client::DownloadMintTask>({unit, 0});
    } catch (...) {

        return error_task();
    }
}

auto OTX::DownloadNym(
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID,
    const identifier::Nym& targetNymID) const -> OTX::BackgroundTask
{
    CHECK_ARGS(localNymID, serverID, targetNymID);

    try {
        start_introduction_server(localNymID);
        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::CheckNymTask>(targetNymID);
    } catch (...) {

        return error_task();
    }
}

auto OTX::DownloadNymbox(
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID) const -> OTX::BackgroundTask
{
    return schedule_download_nymbox(localNymID, serverID);
}

auto OTX::DownloadServerContract(
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID,
    const identifier::Notary& contractID) const -> OTX::BackgroundTask
{
    CHECK_ARGS(localNymID, serverID, contractID);

    try {
        start_introduction_server(localNymID);
        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::DownloadContractTask>({contractID});
    } catch (...) {

        return error_task();
    }
}

auto OTX::DownloadUnitDefinition(
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID,
    const identifier::UnitDefinition& contractID) const -> OTX::BackgroundTask
{
    CHECK_ARGS(localNymID, serverID, contractID);

    try {
        start_introduction_server(localNymID);
        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::DownloadUnitDefinitionTask>(
            {contractID});
    } catch (...) {

        return error_task();
    }
}

auto OTX::extract_payment_data(
    const OTPayment& payment,
    identifier::Nym& nymID,
    identifier::Notary& serverID,
    identifier::UnitDefinition& unitID) const -> bool
{
    if (false == payment.GetRecipientNymID(nymID)) {
        LogError()(OT_PRETTY_CLASS())(
            ": Unable to load recipient nym from instrument.")
            .Flush();

        return false;
    }

    if (false == payment.GetNotaryID(serverID)) {
        LogError()(OT_PRETTY_CLASS())(
            ": Unable to load recipient nym from instrument.")
            .Flush();

        return false;
    }

    OT_ASSERT(false == serverID.empty());

    if (false == payment.GetInstrumentDefinitionID(unitID)) {
        LogError()(OT_PRETTY_CLASS())(
            ": Unable to load recipient nym from instrument.")
            .Flush();

        return false;
    }

    OT_ASSERT(false == unitID.empty());

    return true;
}

auto OTX::error_task() -> OTX::BackgroundTask
{
    BackgroundTask output{0, Future{}};

    return output;
}

void OTX::find_nym(const opentxs::network::zeromq::Message& message) const
{
    const auto body = message.Body();

    if (1 >= body.size()) {
        LogError()(OT_PRETTY_CLASS())("Invalid message").Flush();

        return;
    }

    const auto id = api_.Factory().NymIDFromHash(body.at(1).Bytes());

    if (id.empty()) {
        LogError()(OT_PRETTY_CLASS())("Invalid id").Flush();

        return;
    }

    const auto taskID{next_task_id()};
    missing_nyms_.Push(taskID, id);
    trigger_all();
}

void OTX::find_server(const opentxs::network::zeromq::Message& message) const
{
    const auto body = message.Body();

    if (1 >= body.size()) {
        LogError()(OT_PRETTY_CLASS())("Invalid message").Flush();

        return;
    }

    const auto id = api_.Factory().NotaryIDFromHash(body.at(1).Bytes());

    if (id.empty()) {
        LogError()(OT_PRETTY_CLASS())("Invalid id").Flush();

        return;
    }

    try {
        api_.Wallet().Internal().Server(id);
    } catch (...) {
        const auto taskID{next_task_id()};
        missing_servers_.Push(taskID, id);
        trigger_all();
    }
}

void OTX::find_unit(const opentxs::network::zeromq::Message& message) const
{
    const auto body = message.Body();

    if (1 >= body.size()) {
        LogError()(OT_PRETTY_CLASS())("Invalid message").Flush();

        return;
    }

    const auto id = api_.Factory().UnitIDFromHash(body.at(1).Bytes());

    if (id.empty()) {
        LogError()(OT_PRETTY_CLASS())("Invalid id").Flush();

        return;
    }

    try {
        api_.Wallet().Internal().UnitDefinition(id);

        return;
    } catch (...) {
        const auto taskID{next_task_id()};
        missing_unit_definitions_.Push(taskID, id);
        trigger_all();
    }
}

auto OTX::FindNym(const identifier::Nym& nymID) const -> OTX::BackgroundTask
{
    VALIDATE_NYM(nymID);

    const auto taskID{next_task_id()};
    auto output = start_task(taskID, missing_nyms_.Push(taskID, nymID));
    trigger_all();

    return output;
}

auto OTX::FindNym(
    const identifier::Nym& nymID,
    const identifier::Notary& serverIDHint) const -> OTX::BackgroundTask
{
    VALIDATE_NYM(nymID);

    auto& serverQueue = get_nym_fetch(serverIDHint);
    const auto taskID{next_task_id()};
    auto output = start_task(taskID, serverQueue.Push(taskID, nymID));

    return output;
}

auto OTX::FindServer(const identifier::Notary& serverID) const
    -> OTX::BackgroundTask
{
    VALIDATE_NYM(serverID);

    const auto taskID{next_task_id()};
    auto output = start_task(taskID, missing_servers_.Push(taskID, serverID));

    return output;
}

auto OTX::FindUnitDefinition(const identifier::UnitDefinition& unit) const
    -> OTX::BackgroundTask
{
    VALIDATE_NYM(unit);

    const auto taskID{next_task_id()};
    auto output =
        start_task(taskID, missing_unit_definitions_.Push(taskID, unit));

    return output;
}

auto OTX::finish_task(const TaskID taskID, const bool success, Result&& result)
    const -> bool
{
    if (success) {
        update_task(
            taskID,
            otx::client::ThreadStatus::FINISHED_SUCCESS,
            std::move(result));
    } else {
        update_task(
            taskID,
            otx::client::ThreadStatus::FINISHED_FAILED,
            std::move(result));
    }

    return success;
}

auto OTX::get_introduction_server(const Lock& lock) const -> identifier::Notary
{
    OT_ASSERT(CheckLock(lock, introduction_server_lock_));

    auto keyFound{false};
    auto serverID = String::Factory();
    api_.Config().Internal().Check_str(
        String::Factory(MASTER_SECTION),
        String::Factory(INTRODUCTION_SERVER_KEY),
        serverID,
        keyFound);

    if (serverID->Exists()) {
        return api_.Factory().NotaryIDFromBase58(serverID->Bytes());
    }

    static const auto blank = identifier::Notary{};

    return blank;
}

auto OTX::get_nym_fetch(const identifier::Notary& serverID) const
    -> UniqueQueue<identifier::Nym>&
{
    Lock lock(nym_fetch_lock_);

    return server_nym_fetch_[serverID];
}

auto OTX::get_operations(const ContextID& id) const noexcept(false)
    -> otx::client::implementation::StateMachine&
{
    Lock lock(shutdown_lock_);

    if (shutdown_.load()) { throw; }

    return get_task(id);
}

auto OTX::get_task(const ContextID& id) const
    -> otx::client::implementation::StateMachine&
{
    Lock lock(lock_);
    auto it = operations_.find(id);

    if (operations_.end() == it) {
        auto added = operations_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(id),
            std::forward_as_tuple(
                api_,
                *this,
                running_,
                api_,
                id,
                next_task_id_,
                missing_nyms_,
                outdated_nyms_,
                missing_servers_,
                missing_unit_definitions_,
                reason_));
        it = std::get<0>(added);
    }

    return it->second;
}

auto OTX::InitiateBailment(
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID,
    const identifier::Nym& targetNymID,
    const identifier::UnitDefinition& instrumentDefinitionID,
    const otx::client::SetID setID) const -> OTX::BackgroundTask
{
    CHECK_ARGS(localNymID, serverID, instrumentDefinitionID);
    VALIDATE_NYM(targetNymID);

    start_introduction_server(localNymID);
    const auto nym = api_.Wallet().Nym(localNymID);

    try {
        auto peerrequest = api_.Factory().InternalSession().BailmentRequest(
            nym, targetNymID, instrumentDefinitionID, serverID, reason_);

        if (setID) { setID(peerrequest->ID()); }

        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::PeerRequestTask>(
            {targetNymID, peerrequest.as<contract::peer::Request>()});
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return error_task();
    }
}

auto OTX::InitiateOutbailment(
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID,
    const identifier::Nym& targetNymID,
    const identifier::UnitDefinition& instrumentDefinitionID,
    const Amount amount,
    const UnallocatedCString& message,
    const otx::client::SetID setID) const -> OTX::BackgroundTask
{
    CHECK_ARGS(localNymID, serverID, instrumentDefinitionID);
    VALIDATE_NYM(targetNymID);

    start_introduction_server(localNymID);
    const auto nym = api_.Wallet().Nym(localNymID);

    try {
        auto peerrequest = api_.Factory().InternalSession().OutbailmentRequest(
            nym,
            targetNymID,
            instrumentDefinitionID,
            serverID,
            amount,
            message,
            reason_);

        if (setID) { setID(peerrequest->ID()); }

        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::PeerRequestTask>(
            {targetNymID, peerrequest.as<contract::peer::Request>()});
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return error_task();
    }
}

auto OTX::InitiateRequestConnection(
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID,
    const identifier::Nym& targetNymID,
    const contract::peer::ConnectionInfoType& type,
    const otx::client::SetID setID) const -> OTX::BackgroundTask
{
    CHECK_SERVER(localNymID, serverID);
    VALIDATE_NYM(targetNymID);

    start_introduction_server(localNymID);
    const auto nym = api_.Wallet().Nym(localNymID);

    try {
        auto peerrequest = api_.Factory().InternalSession().ConnectionRequest(
            nym, targetNymID, type, serverID, reason_);

        if (setID) { setID(peerrequest->ID()); }

        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::PeerRequestTask>(
            {targetNymID, peerrequest.as<contract::peer::Request>()});
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return error_task();
    }
}

auto OTX::InitiateStoreSecret(
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID,
    const identifier::Nym& targetNymID,
    const contract::peer::SecretType& type,
    const UnallocatedCString& primary,
    const UnallocatedCString& secondary,
    const otx::client::SetID setID) const -> OTX::BackgroundTask
{
    CHECK_ARGS(localNymID, serverID, targetNymID);

    start_introduction_server(localNymID);
    const auto nym = api_.Wallet().Nym(localNymID);

    if (primary.empty()) {
        LogError()(OT_PRETTY_CLASS())("Warning: primary is empty.").Flush();
    }

    if (secondary.empty()) {
        LogError()(OT_PRETTY_CLASS())("Warning: secondary is empty.").Flush();
    }

    try {
        auto peerrequest = api_.Factory().InternalSession().StoreSecret(
            nym, targetNymID, type, primary, secondary, serverID, reason_);

        if (setID) { setID(peerrequest->ID()); }

        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::PeerRequestTask>(
            {targetNymID, peerrequest.as<contract::peer::Request>()});
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return error_task();
    }
}

auto OTX::IntroductionServer() const -> const identifier::Notary&
{
    Lock lock(introduction_server_lock_);

    if (false == bool(introduction_server_id_)) {
        load_introduction_server(lock);
    }

    return *introduction_server_id_;
}

auto OTX::IssueUnitDefinition(
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID,
    const identifier::UnitDefinition& unitID,
    const UnitType advertise,
    const UnallocatedCString& label) const -> OTX::BackgroundTask
{
    CHECK_ARGS(localNymID, serverID, unitID);

    try {
        start_introduction_server(localNymID);
        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::IssueUnitDefinitionTask>(
            {unitID, label, advertise});
    } catch (...) {

        return error_task();
    }
}

void OTX::load_introduction_server(const Lock& lock) const
{
    OT_ASSERT(CheckLock(lock, introduction_server_lock_));

    introduction_server_id_ =
        std::make_unique<identifier::Notary>(get_introduction_server(lock));
}

auto OTX::MessageContact(
    const identifier::Nym& senderNymID,
    const identifier::Generic& contactID,
    const UnallocatedCString& message,
    const otx::client::SetID setID) const -> OTX::BackgroundTask
{
    CHECK_SERVER(senderNymID, contactID);

    start_introduction_server(senderNymID);
    auto serverID = identifier::Notary{};
    auto recipientNymID = identifier::Nym{};
    const auto canMessage =
        can_message(senderNymID, contactID, recipientNymID, serverID);

    if (otx::client::Messagability::READY != canMessage) {
        return error_task();
    }

    OT_ASSERT(false == serverID.empty());
    OT_ASSERT(false == recipientNymID.empty());

    try {
        auto& queue = get_operations({senderNymID, serverID});

        return queue.StartTask<otx::client::MessageTask>(
            {recipientNymID,
             message,
             std::make_shared<otx::client::SetID>(setID)});
    } catch (...) {

        return error_task();
    }
}

auto OTX::MessageStatus(const TaskID taskID) const
    -> std::pair<otx::client::ThreadStatus, OTX::MessageID>
{
    auto output = std::pair<otx::client::ThreadStatus, OTX::MessageID>{};
    auto& [threadStatus, messageID] = output;
    Lock lock(task_status_lock_);
    threadStatus = status(lock, taskID);

    if (threadStatus == otx::client::ThreadStatus::FINISHED_SUCCESS) {
        auto it = task_message_id_.find(taskID);

        if (task_message_id_.end() != it) {
            messageID = it->second;
            task_message_id_.erase(it);
        }
    }

    return output;
}

auto OTX::NotifyBailment(
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID,
    const identifier::Nym& targetNymID,
    const identifier::UnitDefinition& instrumentDefinitionID,
    const identifier::Generic& requestID,
    const UnallocatedCString& txid,
    const Amount amount,
    const otx::client::SetID setID) const -> OTX::BackgroundTask
{
    CHECK_ARGS(localNymID, serverID, instrumentDefinitionID);
    VALIDATE_NYM(targetNymID);
    VALIDATE_NYM(requestID);

    start_introduction_server(localNymID);
    const auto nym = api_.Wallet().Nym(localNymID);

    try {
        auto peerrequest = api_.Factory().InternalSession().BailmentNotice(
            nym,
            targetNymID,
            instrumentDefinitionID,
            serverID,
            requestID,
            txid,
            amount,
            reason_);

        if (setID) { setID(peerrequest->ID()); }

        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::PeerRequestTask>(
            {targetNymID, peerrequest.as<contract::peer::Request>()});
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return error_task();
    }
}

auto OTX::PayContact(
    const identifier::Nym& senderNymID,
    const identifier::Generic& contactID,
    std::shared_ptr<const OTPayment> payment) const -> OTX::BackgroundTask
{
    CHECK_SERVER(senderNymID, contactID);

    start_introduction_server(senderNymID);
    auto serverID = identifier::Notary{};
    auto recipientNymID = identifier::Nym{};
    const auto canMessage =
        can_message(senderNymID, contactID, recipientNymID, serverID);

    if (otx::client::Messagability::READY != canMessage) {
        return error_task();
    }

    OT_ASSERT(false == serverID.empty());
    OT_ASSERT(false == recipientNymID.empty());

    try {
        auto& queue = get_operations({senderNymID, serverID});

        return queue.StartTask<otx::client::PaymentTask>(
            {recipientNymID, std::shared_ptr<const OTPayment>(payment)});
    } catch (...) {

        return error_task();
    }
}

auto OTX::PayContactCash(
    const identifier::Nym& senderNymID,
    const identifier::Generic& contactID,
    const identifier::Generic& workflowID) const -> OTX::BackgroundTask
{
    CHECK_SERVER(senderNymID, contactID);

    start_introduction_server(senderNymID);
    auto serverID = identifier::Notary{};
    auto recipientNymID = identifier::Nym{};
    const auto canMessage =
        can_message(senderNymID, contactID, recipientNymID, serverID);

    if (otx::client::Messagability::READY != canMessage) {
        return error_task();
    }

    OT_ASSERT(false == serverID.empty());
    OT_ASSERT(false == recipientNymID.empty());

    try {
        auto& queue = get_operations({senderNymID, serverID});

        return queue.StartTask<otx::client::PayCashTask>(
            {recipientNymID, workflowID});
    } catch (...) {

        return error_task();
    }
}

auto OTX::ProcessInbox(
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID,
    const identifier::Generic& accountID) const -> OTX::BackgroundTask
{
    CHECK_ARGS(localNymID, serverID, accountID);

    try {
        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::ProcessInboxTask>({accountID});
    } catch (...) {

        return error_task();
    }
}

void OTX::process_account(const zmq::Message& message) const
{
    const auto body = message.Body();

    OT_ASSERT(2 < body.size());

    const auto accountID =
        api_.Factory().IdentifierFromHash(body.at(1).Bytes());
    const auto balance = factory::Amount(body.at(2));
    LogVerbose()(OT_PRETTY_CLASS())("Account ")(accountID)(" balance: ")(
        balance)
        .Flush();
}

void OTX::process_notification(const zmq::Message& message) const
{
    OT_ASSERT(0 < message.Body().size());

    const auto& frame = message.Body().at(0);
    const auto notification =
        otx::Reply::Factory(api_, proto::Factory<proto::ServerReply>(frame));
    const auto& nymID = notification.Recipient();
    const auto& serverID = notification.Server();

    if (false == valid_context(nymID, serverID)) {
        LogError()(OT_PRETTY_CLASS())(
            ": No context available to handle notification.")
            .Flush();

        return;
    }

    auto context = api_.Wallet().Internal().mutable_ServerContext(
        nymID, serverID, reason_);

    switch (notification.Type()) {
        case otx::ServerReplyType::Push: {
            context.get().ProcessNotification(api_, notification, reason_);
        } break;
        case otx::ServerReplyType::Error:
        case otx::ServerReplyType::Activate:
        default: {
            LogError()(OT_PRETTY_CLASS())(": Unsupported server reply type: ")(
                value(notification.Type()))(".")
                .Flush();
        }
    }
}

auto OTX::publish_messagability(
    const identifier::Nym& sender,
    const identifier::Generic& contact,
    otx::client::Messagability value) const noexcept
    -> otx::client::Messagability
{
    messagability_->Send([&] {
        auto work = opentxs::network::zeromq::tagged_message(
            WorkType::OTXMessagability);
        work.AddFrame(sender);
        work.AddFrame(contact);
        work.AddFrame(value);

        return work;
    }());

    return value;
}

auto OTX::publish_server_registration(
    const identifier::Nym& nymID,
    const identifier::Notary& serverID,
    const bool forcePrimary) const -> bool
{
    OT_ASSERT(false == nymID.empty());
    OT_ASSERT(false == serverID.empty());

    auto nym = api_.Wallet().mutable_Nym(nymID, reason_);

    return nym.AddPreferredOTServer(
        serverID.asBase58(api_.Crypto()), forcePrimary, reason_);
}

auto OTX::PublishServerContract(
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID,
    const identifier::Generic& contractID) const -> OTX::BackgroundTask
{
    CHECK_ARGS(localNymID, serverID, contractID);

    try {
        start_introduction_server(localNymID);
        auto& queue = get_operations({localNymID, serverID});
        // TODO server id type

        return queue.StartTask<otx::client::PublishServerContractTask>(
            {api_.Factory().NotaryIDFromBase58(
                 contractID.asBase58(api_.Crypto())),
             false});
    } catch (...) {

        return error_task();
    }
}

auto OTX::queue_cheque_deposit(
    const identifier::Nym& nymID,
    const Cheque& cheque) const -> bool
{
    auto payment{
        api_.Factory().InternalSession().Payment(String::Factory(cheque))};

    OT_ASSERT(false != bool(payment));

    payment->SetTempValuesFromCheque(cheque);

    if (cheque.GetRecipientNymID().empty()) {
        payment->SetTempRecipientNymID(nymID);
    }

    std::shared_ptr<OTPayment> ppayment{payment.release()};
    const auto task = DepositPayment(nymID, ppayment);
    const auto taskID = std::get<0>(task);

    return (0 != taskID);
}

void OTX::Refresh() const
{
    refresh_accounts();
    refresh_contacts();
    ++refresh_counter_;
    trigger_all();
}

auto OTX::RefreshCount() const -> std::uint64_t
{
    return refresh_counter_.load();
}

auto OTX::refresh_accounts() const -> bool
{
    LogVerbose()(OT_PRETTY_CLASS())("Begin").Flush();
    const auto serverList = api_.Wallet().ServerList();
    const auto accounts = api_.Storage().AccountList();

    for (const auto& server : serverList) {
        SHUTDOWN_OTX();

        const auto serverID = api_.Factory().NotaryIDFromBase58(server.first);
        LogDetail()(OT_PRETTY_CLASS())("Considering server ")(serverID).Flush();

        for (const auto& nymID : api_.Wallet().LocalNyms()) {
            SHUTDOWN_OTX();
            auto logStr = String::Factory(": Nym ");
            logStr->Concatenate(String::Factory(nymID.asBase58(api_.Crypto())));
            const bool registered =
                api_.InternalClient().OTAPI().IsNym_RegisteredAtServer(
                    nymID, serverID);

            if (registered) {
                static auto is = String::Factory(UnallocatedCString{" is "});
                logStr->Concatenate(is);
                try {
                    auto& queue = get_operations({nymID, serverID});
                    queue.StartTask<otx::client::DownloadNymboxTask>({});
                } catch (...) {

                    return false;
                }
            } else {
                static auto is_not =
                    String::Factory(UnallocatedCString{" is not "});
                logStr->Concatenate(is_not);
            }

            static auto registered_here =
                String::Factory(UnallocatedCString{" registered here."});
            logStr->Concatenate(registered_here);
            LogDetail()(OT_PRETTY_CLASS())(logStr.get()).Flush();
        }
    }

    SHUTDOWN_OTX();

    for (const auto& it : accounts) {
        SHUTDOWN_OTX();
        const auto accountID = api_.Factory().IdentifierFromBase58(it.first);
        const auto nymID = api_.Storage().AccountOwner(accountID);
        const auto serverID = api_.Storage().AccountServer(accountID);
        LogDetail()(OT_PRETTY_CLASS())("Account ")(accountID)(": ")(
            "  * Owned by nym: ")(nymID)("  * "
                                         "On "
                                         "server"
                                         ":"
                                         " ")(serverID)
            .Flush();
        try {
            auto& queue = get_operations({nymID, serverID});

            if (0 == queue.StartTask<otx::client::ProcessInboxTask>({accountID})
                         .first) {

                return false;
            }
        } catch (...) {

            return false;
        }
    }

    LogVerbose()(OT_PRETTY_CLASS())("End").Flush();

    return true;
}

auto OTX::refresh_contacts() const -> bool
{
    for (const auto& it : api_.Contacts().ContactList()) {
        SHUTDOWN_OTX();

        const auto& contactID = it.first;
        LogVerbose()(OT_PRETTY_CLASS())("Considering contact: ")(contactID)
            .Flush();
        const auto contact = api_.Contacts().Contact(
            api_.Factory().IdentifierFromBase58(contactID));

        OT_ASSERT(contact);

        const auto now = std::time(nullptr);
        const std::chrono::seconds interval(now - contact->LastUpdated());
        const std::chrono::hours limit(24 * CONTACT_REFRESH_DAYS);
        const auto nymList = contact->Nyms();

        if (nymList.empty()) {
            LogVerbose()(OT_PRETTY_CLASS())(
                ": No nyms associated with this contact.")
                .Flush();

            continue;
        }

        for (const auto& nymID : nymList) {
            SHUTDOWN_OTX();

            const auto nym = api_.Wallet().Nym(nymID);
            LogVerbose()(OT_PRETTY_CLASS())("Considering nym: ")(nymID).Flush();

            if (!nym) {
                LogVerbose()(OT_PRETTY_CLASS())(
                    ": We don't have credentials for this nym. "
                    " Will search on all servers.")
                    .Flush();
                const auto taskID{next_task_id()};
                missing_nyms_.Push(taskID, nymID);

                continue;
            }

            if (interval > limit) {
                LogVerbose()(OT_PRETTY_CLASS())(
                    ": Hours since last update "
                    "(")(interval.count())(") exceeds "
                                           "the limit "
                                           "(")(limit.count())(")")
                    .Flush();
                // TODO add a method to Contact that returns the list of
                // servers
                const auto data = contact->Data();

                if (false == bool(data)) { continue; }

                const auto serverGroup = data->Group(
                    identity::wot::claim::SectionType::Communication,
                    identity::wot::claim::ClaimType::Opentxs);

                if (false == bool(serverGroup)) {

                    const auto taskID{next_task_id()};
                    outdated_nyms_.Push(taskID, nymID);
                    continue;
                }

                for (const auto& [claimID, item] : *serverGroup) {
                    SHUTDOWN_OTX();
                    OT_ASSERT(item);

                    const auto& notUsed [[maybe_unused]] = claimID;
                    const auto serverID =
                        api_.Factory().NotaryIDFromBase58(item->Value());

                    if (serverID.empty()) { continue; }

                    LogVerbose()(OT_PRETTY_CLASS())("Will download nym ")(
                        nymID)(" from "
                               "server ")(serverID)
                        .Flush();
                    auto& serverQueue = get_nym_fetch(serverID);
                    const auto taskID{next_task_id()};
                    serverQueue.Push(taskID, nymID);
                }
            } else {
                LogVerbose()(OT_PRETTY_CLASS())(": No need to update this nym.")
                    .Flush();
            }
        }
    }

    return true;
}

auto OTX::RegisterAccount(
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID,
    const identifier::UnitDefinition& unitID,
    const UnallocatedCString& label) const -> OTX::BackgroundTask
{
    return schedule_register_account(localNymID, serverID, unitID, label);
}

auto OTX::RegisterNym(
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID,
    const bool resync) const -> OTX::BackgroundTask
{
    CHECK_SERVER(localNymID, serverID);

    try {
        start_introduction_server(localNymID);
        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::RegisterNymTask>({resync});
    } catch (...) {

        return error_task();
    }
}

auto OTX::RegisterNymPublic(
    const identifier::Nym& nymID,
    const identifier::Notary& serverID,
    const bool setContactData,
    const bool forcePrimary,
    const bool resync) const -> OTX::BackgroundTask
{
    CHECK_SERVER(nymID, serverID);

    start_introduction_server(nymID);

    if (setContactData) {
        publish_server_registration(nymID, serverID, forcePrimary);
    }

    return RegisterNym(nymID, serverID, resync);
}

auto OTX::SetIntroductionServer(const contract::Server& contract) const
    -> identifier::Notary
{
    Lock lock(introduction_server_lock_);

    return set_introduction_server(lock, contract);
}

auto OTX::schedule_download_nymbox(
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID) const -> OTX::BackgroundTask
{
    CHECK_SERVER(localNymID, serverID);

    try {
        start_introduction_server(localNymID);
        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::DownloadNymboxTask>({});
    } catch (...) {

        return error_task();
    }
}

auto OTX::schedule_register_account(
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID,
    const identifier::UnitDefinition& unitID,
    const UnallocatedCString& label) const -> OTX::BackgroundTask
{
    CHECK_ARGS(localNymID, serverID, unitID);

    try {
        start_introduction_server(localNymID);
        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::RegisterAccountTask>(
            {label, unitID});
    } catch (...) {

        return error_task();
    }
}

auto OTX::SendCheque(
    const identifier::Nym& localNymID,
    const identifier::Generic& sourceAccountID,
    const identifier::Generic& recipientContactID,
    const Amount value,
    const UnallocatedCString& memo,
    const Time validFrom,
    const Time validTo) const -> OTX::BackgroundTask
{
    CHECK_ARGS(localNymID, sourceAccountID, recipientContactID);

    start_introduction_server(localNymID);
    auto serverID = identifier::Notary{};
    auto recipientNymID = identifier::Nym{};
    const auto canMessage =
        can_message(localNymID, recipientContactID, recipientNymID, serverID);
    const bool closeEnough =
        (otx::client::Messagability::READY == canMessage) ||
        (otx::client::Messagability::UNREGISTERED == canMessage);

    if (false == closeEnough) {
        LogError()(OT_PRETTY_CLASS())("Unable to message contact.").Flush();

        return error_task();
    }

    if (0 >= value) {
        LogError()(OT_PRETTY_CLASS())("Invalid amount.").Flush();

        return error_task();
    }

    auto account = api_.Wallet().Internal().Account(sourceAccountID);

    if (false == bool(account)) {
        LogError()(OT_PRETTY_CLASS())("Invalid account.").Flush();

        return error_task();
    }

    try {
        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::SendChequeTask>(
            {sourceAccountID, recipientNymID, value, memo, validFrom, validTo});
    } catch (...) {

        return error_task();
    }
}

auto OTX::SendExternalTransfer(
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID,
    const identifier::Generic& sourceAccountID,
    const identifier::Generic& targetAccountID,
    const Amount& value,
    const UnallocatedCString& memo) const -> OTX::BackgroundTask
{
    CHECK_ARGS(localNymID, serverID, targetAccountID);
    VALIDATE_NYM(sourceAccountID);

    auto sourceAccount = api_.Wallet().Internal().Account(sourceAccountID);

    if (false == bool(sourceAccount)) {
        LogError()(OT_PRETTY_CLASS())("Invalid source account.").Flush();

        return error_task();
    }

    if (sourceAccount.get().GetNymID() != localNymID) {
        LogError()(OT_PRETTY_CLASS())("Wrong owner on source account.").Flush();

        return error_task();
    }

    if (sourceAccount.get().GetRealNotaryID() != serverID) {
        LogError()(OT_PRETTY_CLASS())("Wrong notary on source account.")
            .Flush();

        return error_task();
    }

    try {
        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::SendTransferTask>(
            {sourceAccountID, targetAccountID, value, memo});
    } catch (...) {

        return error_task();
    }
}

auto OTX::SendTransfer(
    const identifier::Nym& localNymID,
    const identifier::Notary& serverID,
    const identifier::Generic& sourceAccountID,
    const identifier::Generic& targetAccountID,
    const Amount& value,
    const UnallocatedCString& memo) const -> OTX::BackgroundTask
{
    CHECK_ARGS(localNymID, serverID, targetAccountID);
    VALIDATE_NYM(sourceAccountID);

    auto sourceAccount = api_.Wallet().Internal().Account(sourceAccountID);

    if (false == bool(sourceAccount)) {
        LogError()(OT_PRETTY_CLASS())("Invalid source account.").Flush();

        return error_task();
    }

    if (sourceAccount.get().GetNymID() != localNymID) {
        LogError()(OT_PRETTY_CLASS())("Wrong owner on source account.").Flush();

        return error_task();
    }

    if (sourceAccount.get().GetRealNotaryID() != serverID) {
        LogError()(OT_PRETTY_CLASS())("Wrong notary on source account.")
            .Flush();

        return error_task();
    }

    try {
        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::SendTransferTask>(
            {sourceAccountID, targetAccountID, value, memo});
    } catch (...) {

        return error_task();
    }
}

void OTX::set_contact(
    const identifier::Nym& nymID,
    const identifier::Notary& serverID) const
{
    auto nym = api_.Wallet().mutable_Nym(nymID, reason_);
    const auto server = nym.PreferredOTServer();

    if (server.empty()) {
        nym.AddPreferredOTServer(
            serverID.asBase58(api_.Crypto()), true, reason_);
    }
}

auto OTX::set_introduction_server(
    const Lock& lock,
    const contract::Server& contract) const -> identifier::Notary
{
    OT_ASSERT(CheckLock(lock, introduction_server_lock_));

    try {
        const auto& config = api_.Config().Internal();
        auto serialized = proto::ServerContract{};

        if (false == contract.Serialize(serialized, true)) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed to serialize server contract.")
                .Flush();
        }

        const auto instantiated = api_.Wallet().Internal().Server(serialized);
        const auto id =
            api_.Factory().Internal().NotaryIDConvertSafe(instantiated->ID());
        introduction_server_id_ = std::make_unique<identifier::Notary>(id);

        OT_ASSERT(introduction_server_id_);

        bool dontCare = false;
        const bool set = config.Set_str(
            String::Factory(MASTER_SECTION),
            String::Factory(INTRODUCTION_SERVER_KEY),
            String::Factory(id),
            dontCare);

        OT_ASSERT(set);

        if (false == config.Save()) {
            LogAbort()(OT_PRETTY_CLASS())("failed to save config file").Abort();
        }

        return id;
    } catch (...) {
        return identifier::Notary{};
    }
}

void OTX::start_introduction_server(const identifier::Nym& nymID) const
{
    try {
        const auto& serverID = IntroductionServer();

        if (serverID.empty()) { return; }

        auto& queue = get_operations({nymID, serverID});
        queue.StartTask<otx::client::DownloadNymboxTask>({});
    } catch (...) {

        return;
    }
}

auto OTX::start_task(const TaskID taskID, bool success) const
    -> OTX::BackgroundTask
{
    if (0 == taskID) {
        LogTrace()(OT_PRETTY_CLASS())("Empty task ID").Flush();

        return error_task();
    }

    if (false == success) {
        LogTrace()(OT_PRETTY_CLASS())("Task already queued").Flush();

        return error_task();
    }

    return add_task(taskID, otx::client::ThreadStatus::RUNNING);
}

void OTX::StartIntroductionServer(const identifier::Nym& localNymID) const
{
    start_introduction_server(localNymID);
}

auto OTX::status(const Lock& lock, const TaskID taskID) const
    -> otx::client::ThreadStatus
{
    OT_ASSERT(CheckLock(lock, task_status_lock_));

    if (!running_) { return otx::client::ThreadStatus::SHUTDOWN; }

    auto it = task_status_.find(taskID);

    if (task_status_.end() == it) { return otx::client::ThreadStatus::Error; }

    const auto output = it->second.first;
    const bool success =
        (otx::client::ThreadStatus::FINISHED_SUCCESS == output);
    const bool failed = (otx::client::ThreadStatus::FINISHED_FAILED == output);
    const bool finished = (success || failed);

    if (finished) { task_status_.erase(it); }

    return output;
}

auto OTX::Status(const TaskID taskID) const -> otx::client::ThreadStatus
{
    Lock lock(task_status_lock_);

    return status(lock, taskID);
}

void OTX::trigger_all() const
{
    Lock lock(shutdown_lock_);

    for (const auto& [id, queue] : operations_) {
        if (false == queue.Trigger()) { return; }
    }
}

void OTX::update_task(
    const TaskID taskID,
    const otx::client::ThreadStatus status,
    Result&& result) const noexcept
{
    if (0 == taskID) { return; }

    Lock lock(task_status_lock_);

    if (0 == task_status_.count(taskID)) { return; }

    try {
        auto& row = task_status_.at(taskID);
        auto& [state, promise] = row;
        state = status;
        bool value{false};
        bool publish{false};

        switch (status) {
            case otx::client::ThreadStatus::FINISHED_SUCCESS: {
                value = true;
                publish = true;
                promise.set_value(std::move(result));
            } break;
            case otx::client::ThreadStatus::FINISHED_FAILED: {
                value = false;
                publish = true;
                promise.set_value(std::move(result));
            } break;
            case otx::client::ThreadStatus::SHUTDOWN: {
                Result cancel{otx::LastReplyStatus::Unknown, nullptr};
                promise.set_value(std::move(cancel));
            } break;
            case otx::client::ThreadStatus::Error:
            case otx::client::ThreadStatus::RUNNING:
            default: {
            }
        }

        if (publish) {
            task_finished_->Send([&] {
                auto work = opentxs::network::zeromq::tagged_message(
                    WorkType::OTXTaskComplete);
                work.AddFrame(taskID);
                work.AddFrame(value);

                return work;
            }());
        }
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())(
            "Tried to finish an already-finished task (")(taskID)(")")
            .Flush();
    }
}

auto OTX::valid_account(
    const OTPayment& payment,
    const identifier::Nym& recipient,
    const identifier::Notary& paymentServerID,
    const identifier::UnitDefinition& paymentUnitID,
    const identifier::Generic& accountIDHint,
    identifier::Generic& depositAccount) const -> otx::client::Depositability
{
    UnallocatedSet<identifier::Generic> matchingAccounts{};

    for (const auto& it : api_.Storage().AccountList()) {
        const auto accountID = api_.Factory().IdentifierFromBase58(it.first);
        const auto nymID = api_.Storage().AccountOwner(accountID);
        const auto serverID = api_.Storage().AccountServer(accountID);
        const auto unitID = api_.Storage().AccountContract(accountID);

        if (nymID != recipient) { continue; }

        if (serverID != paymentServerID) { continue; }

        if (unitID != paymentUnitID) { continue; }

        matchingAccounts.emplace(accountID);
    }

    if (accountIDHint.empty()) {
        if (0 == matchingAccounts.size()) {

            return otx::client::Depositability::NO_ACCOUNT;
        } else if (1 == matchingAccounts.size()) {
            depositAccount.Assign(*matchingAccounts.begin());

            return otx::client::Depositability::READY;
        } else {

            return otx::client::Depositability::ACCOUNT_NOT_SPECIFIED;
        }
    }

    if (0 == matchingAccounts.size()) {

        return otx::client::Depositability::NO_ACCOUNT;
    } else if (1 == matchingAccounts.count(accountIDHint)) {
        depositAccount.Assign(accountIDHint);

        return otx::client::Depositability::READY;
    } else {

        return otx::client::Depositability::WRONG_ACCOUNT;
    }
}

auto OTX::valid_context(
    const identifier::Nym& nymID,
    const identifier::Notary& serverID) const -> bool
{
    const auto nyms = api_.Wallet().LocalNyms();

    if (0 == nyms.count(nymID)) {
        LogError()(OT_PRETTY_CLASS())("Nym ")(
            nymID)(" does not belong to this wallet.")
            .Flush();

        return false;
    }

    if (serverID.empty()) {
        LogError()(OT_PRETTY_CLASS())("Invalid server.").Flush();

        return false;
    }

    const auto context =
        api_.Wallet().Internal().ServerContext(nymID, serverID);

    if (false == bool(context)) {
        LogError()(OT_PRETTY_CLASS())("Context does not exist.").Flush();

        return false;
    }

    if (0 == context->Request()) {
        LogError()(OT_PRETTY_CLASS())("Nym is not registered at this server.")
            .Flush();

        return false;
    }

    return true;
}

auto OTX::valid_recipient(
    const OTPayment& payment,
    const identifier::Nym& specified,
    const identifier::Nym& recipient) const -> otx::client::Depositability
{
    if (specified.empty()) {
        LogError()(OT_PRETTY_CLASS())("Payment can be accepted by any nym.")
            .Flush();

        return otx::client::Depositability::READY;
    }

    if (recipient == specified) { return otx::client::Depositability::READY; }

    return otx::client::Depositability::WRONG_RECIPIENT;
}

auto OTX::WithdrawCash(
    const identifier::Nym& nymID,
    const identifier::Notary& serverID,
    const identifier::Generic& account,
    const Amount amount) const -> OTX::BackgroundTask
{
    CHECK_ARGS(nymID, serverID, account);

    try {
        start_introduction_server(nymID);
        auto& queue = get_operations({nymID, serverID});

        return queue.StartTask<otx::client::WithdrawCashTask>(
            {account, amount});
    } catch (...) {

        return error_task();
    }
}

OTX::~OTX()
{
    account_subscriber_->Close();
    notification_listener_->Close();
    find_unit_listener_->Close();
    find_server_listener_->Close();
    find_nym_listener_->Close();

    Lock lock(shutdown_lock_);
    shutdown_.store(true);
    lock.unlock();
    UnallocatedVector<otx::client::implementation::StateMachine::WaitFuture>
        futures{};

    for (const auto& [id, queue] : operations_) {
        futures.emplace_back(queue.Stop());
    }

    for (const auto& future : futures) { future.get(); }

    for (auto& it : task_status_) {
        auto& promise = it.second.second;

        try {
            promise.set_value(error_result());
        } catch (...) {
        }
    }
}
}  // namespace opentxs::api::session::imp

#undef SHUTDOWN_OTX
#undef YIELD_OTX
#undef CHECK_ARGS
#undef CHECK_SERVER
#undef VALIDATE_NYM
