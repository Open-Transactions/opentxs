// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "otx/server/MessageProcessor.hpp"  // IWYU pragma: associated

#include <OTXPush.pb.h>
#include <ServerReply.pb.h>
#include <ServerRequest.pb.h>
#include <chrono>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>

#include "Proto.hpp"
#include "Proto.tpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/api/session/notary/Notary.hpp"
#include "internal/network/zeromq/Batch.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/message/Message.hpp"  // IWYU pragma: keep
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/otx/common/Message.hpp"
#include "internal/otx/server/Types.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/verify/ServerRequest.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/Size.hpp"
#include "internal/util/Thread.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Notary.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/otx/Reply.hpp"
#include "opentxs/otx/Request.hpp"
#include "opentxs/otx/ServerReplyType.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/WorkType.hpp"
#include "otx/server/Server.hpp"
#include "otx/server/UserCommandProcessor.hpp"

namespace opentxs::server
{
auto print(Jobs job) noexcept -> std::string_view
{
    try {
        using Job = Jobs;
        static const auto map = Map<Job, CString>{
            {Job::shutdown, "shutdown"},
            {Job::init, "init"},
            {Job::statemachine, "statemachine"},
        };

        return map.at(job);
    } catch (...) {
        LogError()(__FUNCTION__)("invalid Jobs: ")(
            static_cast<OTZMQWorkType>(job))
            .Flush();

        OT_FAIL;
    }
}
}  // namespace opentxs::server

namespace opentxs::server
{
MessageProcessor::Imp::Imp(
    Server& server,
    const PasswordPrompt& reason) noexcept
    : api_(server.API())
    , server_(server)
    , reason_(reason)
    , running_(true)
    , zmq_handle_(api_.Network().ZeroMQ().Internal().MakeBatch(
          {
              zmq::socket::Type::Router,  // NOTE frontend_
              zmq::socket::Type::Pull,    // NOTE notification_
          },
          "server::MessageProcessor"))
    , zmq_batch_(zmq_handle_.batch_)
    , frontend_([&]() -> auto& {
        auto& out = zmq_batch_.sockets_.at(0);
        const auto rc = out.SetExposedUntrusted();

        OT_ASSERT(rc);

        return out;
    }())
    , notification_(zmq_batch_.sockets_.at(1))
    , zmq_thread_(nullptr)
    , frontend_id_(frontend_.ID())
    , thread_()
    , counter_lock_()
    , drop_incoming_(0)
    , drop_outgoing_(0)
    , active_connections_()
    , connection_map_lock_()
{
    zmq_batch_.listen_callbacks_.emplace_back(zmq::ListenCallback::Factory(
        [this](auto&& m) { old_pipeline(std::move(m)); }));
    auto rc = notification_.Bind(
        api_.Endpoints().Internal().PushNotification().data());

    OT_ASSERT(rc);

    zmq_thread_ = api_.Network().ZeroMQ().Internal().Start(
        zmq_batch_.id_,
        {
            {frontend_.ID(),
             &frontend_,
             [id = frontend_.ID(),
              &cb = zmq_batch_.listen_callbacks_.at(0).get()](auto&& m) {
                 m.Internal().Prepend(id);
                 cb.Process(std::move(m));
             }},
            {notification_.ID(),
             &notification_,
             [id = notification_.ID(),
              &cb = zmq_batch_.listen_callbacks_.at(0).get()](auto&& m) {
                 m.Internal().Prepend(id);
                 cb.Process(std::move(m));
             }},
        });

    OT_ASSERT(nullptr != zmq_thread_);

    LogTrace()(OT_PRETTY_CLASS())("using ZMQ batch ")(zmq_batch_.id_).Flush();
}

auto MessageProcessor::Imp::associate_connection(
    const bool oldFormat,
    const identifier::Nym& nym,
    const Data& connection) noexcept -> void
{
    if (nym.empty()) { return; }
    if (connection.empty()) { return; }

    const auto changed = [&] {
        auto& map = active_connections_;
        auto lock = eLock{connection_map_lock_};
        auto output{false};

        if (auto it = map.find(nym); map.end() == it) {
            map.try_emplace(nym, connection, oldFormat);
            output = true;
        } else {
            auto& [id, format] = it->second;

            if (id != connection) {
                output = true;
                id = connection;
            }

            if (format != oldFormat) {
                output = true;
                format = oldFormat;
            }
        }

        return output;
    }();

    if (changed) {
        LogDetail()(OT_PRETTY_CLASS())("Nym ")(
            nym)(" is available via connection ")(connection.asHex())
            .Flush();
    }
}

auto MessageProcessor::Imp::cleanup() noexcept -> void
{
    running_ = false;
    zmq_handle_.Release();
}

auto MessageProcessor::Imp::DropIncoming(const int count) const noexcept -> void
{
    Lock lock(counter_lock_);
    drop_incoming_ = count;
}

auto MessageProcessor::Imp::DropOutgoing(const int count) const noexcept -> void
{
    Lock lock(counter_lock_);
    drop_outgoing_ = count;
}

auto MessageProcessor::Imp::extract_proto(
    const zmq::Frame& incoming) const noexcept -> proto::ServerRequest
{
    return proto::Factory<proto::ServerRequest>(incoming);
}

auto MessageProcessor::Imp::get_connection(
    const network::zeromq::Message& incoming) noexcept -> ByteArray
{
    auto output = ByteArray{};
    const auto header = incoming.Header();

    if (0 < header.size()) {
        const auto& frame = header.at(0);
        output.Assign(frame.Bytes());
    }

    return output;
}

auto MessageProcessor::Imp::init(
    const bool inproc,
    const int port,
    const Secret& privkey) noexcept(false) -> void
{
    if (port == 0) { OT_FAIL; }

    api_.Network().ZeroMQ().Internal().Modify(
        frontend_id_,
        [this, inproc, port, key = OTSecret{privkey}](auto& socket) {
            auto set = socket.SetPrivateKey(key->Bytes());

            OT_ASSERT(set);

            set = socket.SetZAPDomain(zap_domain_);

            OT_ASSERT(set);

            auto endpoint = std::stringstream{};

            if (inproc) {
                endpoint << api_.InternalNotary().InprocEndpoint();
                endpoint << ':';
            } else {
                endpoint << UnallocatedCString("tcp://*:");
            }

            endpoint << std::to_string(port);
            const auto bound = socket.Bind(endpoint.str().c_str());

            if (false == bound) {
                throw std::invalid_argument("Cannot bind to endpoint");
            }

            LogConsole()("Bound to endpoint: ")(endpoint.str()).Flush();
        });
}

auto MessageProcessor::Imp::old_pipeline(zmq::Message&& message) noexcept
    -> void
{
    const auto isFrontend = [&] {
        const auto header = message.Header();

        OT_ASSERT(0 < header.size());

        const auto socket = message.Internal().ExtractFront();
        const auto output = (frontend_id_ == socket.as<zmq::SocketID>());

        return output;
    }();

    if (isFrontend) {
        process_frontend(std::move(message));
    } else {
        process_notification(std::move(message));
    }
}

auto MessageProcessor::Imp::process_backend(
    const bool tagged,
    zmq::Message&& incoming) noexcept -> network::zeromq::Message
{
    auto reply = UnallocatedCString{};
    const auto error = [&] {
        // ProcessCron and process_backend must not run simultaneously
        auto lock = Lock{lock_};
        const auto request = [&] {
            auto out = UnallocatedCString{};
            const auto body = incoming.Body();

            if (0u < body.size()) { out = body.at(0).Bytes(); }

            return out;
        }();

        return process_message(request, reply);
    }();

    if (error) { reply = ""; }

    auto output = network::zeromq::reply_to_message(std::move(incoming));

    if (tagged) { output.AddFrame(WorkType::OTXLegacyXML); }

    output.AddFrame(reply);

    return output;
}

auto MessageProcessor::Imp::process_command(
    const proto::ServerRequest& serialized,
    identifier::Nym& nymID) noexcept -> bool
{
    const auto allegedNymID = api_.Factory().NymIDFromBase58(serialized.nym());
    const auto nym = api_.Wallet().Nym(allegedNymID);

    if (false == bool(nym)) {
        LogError()(OT_PRETTY_CLASS())("Nym is not yet registered.").Flush();

        return true;
    }

    auto request = otx::Request::Factory(api_, serialized);

    if (request.Validate()) {
        nymID.Assign(request.Initiator());
    } else {
        LogError()(OT_PRETTY_CLASS())("Invalid request.").Flush();

        return false;
    }

    // TODO look at the request type and do some stuff

    return true;
}

auto MessageProcessor::Imp::process_frontend(zmq::Message&& message) noexcept
    -> void
{
    const auto drop = [&] {
        auto lock = Lock{counter_lock_};

        if (0 < drop_incoming_) {
            LogConsole()(OT_PRETTY_CLASS())(
                "Dropping incoming message for testing.")
                .Flush();
            --drop_incoming_;

            return true;
        } else {

            return false;
        }
    }();

    if (drop) { return; }

    const auto id = get_connection(message);
    const auto body = message.Body();

    if (2u > body.size()) {
        process_legacy(id, false, std::move(message));

        return;
    }

    try {
        auto oldProtoFormat = false;
        const auto type = [&] {
            if ((2u == body.size()) && (0u == body.at(1).size())) {
                oldProtoFormat = true;

                return WorkType::OTXRequest;
            }

            try {

                return body.at(0).as<WorkType>();
            } catch (...) {
                throw std::runtime_error{"Invalid message type"};
            }
        }();

        switch (type) {
            case WorkType::OTXRequest: {
                process_proto(id, oldProtoFormat, std::move(message));
            } break;
            case WorkType::OTXLegacyXML: {
                process_legacy(id, true, std::move(message));
            } break;
            default: {
                throw std::runtime_error{"Unsupported message type"};
            }
        }
    } catch (const std::exception& e) {
        LogConsole()(e.what())(" from ").asHex(id).Flush();
    }
}

auto MessageProcessor::Imp::process_internal(zmq::Message&& message) noexcept
    -> void
{
    const auto drop = [&] {
        auto lock = Lock{counter_lock_};

        if (0 < drop_outgoing_) {
            LogConsole()(OT_PRETTY_CLASS())(
                "Dropping outgoing message for testing.")
                .Flush();
            --drop_outgoing_;

            return true;
        } else {

            return false;
        }
    }();

    if (drop) { return; }

    const auto sent =
        frontend_.SendExternal(std::move(message), __FILE__, __LINE__);

    if (sent) {
        LogTrace()(OT_PRETTY_CLASS())("Reply message delivered.").Flush();
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed to send reply message.").Flush();
    }
}

auto MessageProcessor::Imp::process_legacy(
    const Data& id,
    const bool tagged,
    network::zeromq::Message&& incoming) noexcept -> void
{
    LogTrace()(OT_PRETTY_CLASS())("Processing request via ")(id.asHex())
        .Flush();
    process_internal(process_backend(tagged, std::move(incoming)));
}

auto MessageProcessor::Imp::process_message(
    const UnallocatedCString& messageString,
    UnallocatedCString& reply) noexcept -> bool
{
    if (messageString.size() < 1) { return true; }

    try {
        auto armored = Armored::Factory();
        armored->MemSet(messageString.data(), shorten(messageString.size()));
        auto serialized = String::Factory();
        armored->GetString(serialized);
        auto request{api_.Factory().InternalSession().Message()};

        if (false == serialized->Exists()) {
            LogError()(OT_PRETTY_CLASS())("Empty serialized request.").Flush();

            return true;
        }

        if (false == request->LoadContractFromString(serialized)) {
            LogError()(OT_PRETTY_CLASS())("Failed to deserialized request.")
                .Flush();

            return true;
        }

        auto replymsg{api_.Factory().InternalSession().Message()};

        OT_ASSERT(false != bool(replymsg));

        const bool processed =
            server_.CommandProcessor().ProcessUserCommand(*request, *replymsg);

        if (false == processed) {
            LogDetail()(OT_PRETTY_CLASS())("Failed to process user command ")(
                request->m_strCommand.get())
                .Flush();
            LogVerbose()(OT_PRETTY_CLASS())(String::Factory(*request).get())
                .Flush();
        } else {
            LogDetail()(OT_PRETTY_CLASS())(
                "Successfully processed user command ")(
                request->m_strCommand.get())
                .Flush();
        }

        auto serializedReply = String::Factory(*replymsg);

        if (false == serializedReply->Exists()) {
            LogError()(OT_PRETTY_CLASS())("Failed to serialize reply.").Flush();

            return true;
        }

        auto armoredReply = Armored::Factory(serializedReply);

        if (false == armoredReply->Exists()) {
            LogError()(OT_PRETTY_CLASS())("Failed to armor reply.").Flush();

            return true;
        }

        reply.assign(armoredReply->Get(), armoredReply->GetLength());

        return false;
    } catch (...) {

        return true;
    }
}

auto MessageProcessor::Imp::process_notification(
    zmq::Message&& incoming) noexcept -> void
{
    if (2 != incoming.Body().size()) {
        LogError()(OT_PRETTY_CLASS())("Invalid message.").Flush();

        return;
    }

    const auto nymID =
        api_.Factory().NymIDFromBase58(incoming.Body().at(0).Bytes());
    const auto& data = query_connection(nymID);
    const auto& [connection, oldFormat] = data;

    if (connection.empty()) {
        LogDebug()(OT_PRETTY_CLASS())("No notification channel available for ")(
            nymID)(".")
            .Flush();

        return;
    }

    const auto nym = api_.Wallet().Nym(server_.GetServerNym().ID());

    OT_ASSERT(nym);

    const auto& payload = incoming.Body().at(1);
    auto message = otx::Reply::Factory(
        api_,
        nym,
        nymID,
        server_.GetServerID(),
        otx::ServerReplyType::Push,
        true,
        false,
        reason_,
        proto::DynamicFactory<proto::OTXPush>(payload));

    OT_ASSERT(message.Validate());

    auto serialized = proto::ServerReply{};

    if (false == message.Serialize(serialized)) {
        LogVerbose()(OT_PRETTY_CLASS())("Failed to serialize reply.").Flush();

        return;
    }

    const auto reply = api_.Factory().InternalSession().Data(serialized);
    const auto sent = frontend_.SendExternal(
        [&] {
            auto out = zmq::Message{};
            out.AddFrame(data.first);
            out.StartBody();

            if (data.second) {
                out.AddFrame(reply);
                out.AddFrame();
            } else {
                out.AddFrame(WorkType::OTXPush);
                out.AddFrame(reply);
            }

            return out;
        }(),
        __FILE__,
        __LINE__);

    if (sent) {
        LogVerbose()(OT_PRETTY_CLASS())("Push notification for ")(
            nymID)(" delivered via ")
            .asHex(connection)
            .Flush();
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed to deliver push notifcation "
                                      "for ")(nymID)(" via ")
            .asHex(connection)
            .Flush();
    }
}

auto MessageProcessor::Imp::process_proto(
    const Data& id,
    const bool oldFormat,
    network::zeromq::Message&& incoming) noexcept -> void
{
    LogTrace()(OT_PRETTY_CLASS())("Processing request via ")(id.asHex())
        .Flush();
    const auto body = incoming.Body();
    const auto& payload = [&]() -> auto&
    {
        if (oldFormat) {

            return body.at(0);
        } else {

            return body.at(1);
        }
    }
    ();
    const auto command = extract_proto(payload);

    if (false == proto::Validate(command, VERBOSE)) {
        LogError()(OT_PRETTY_CLASS())("Invalid otx request.").Flush();

        return;
    }

    auto nymID = identifier::Nym{};
    const auto valid = process_command(command, nymID);

    if (valid && (false == id.empty())) {
        associate_connection(oldFormat, nymID, id);
    }
}

auto MessageProcessor::Imp::query_connection(const identifier::Nym& id) noexcept
    -> const ConnectionData&
{
    auto lock = sLock{connection_map_lock_};

    try {

        return active_connections_.at(id);
    } catch (...) {
        static const auto blank = ConnectionData{api_.Factory().Data(), true};

        return blank;
    }
}

auto MessageProcessor::Imp::run() noexcept -> void
{
    SetThisThreadsName("MessageProcessor");

    while (running_.load()) {
        // timeout is the time left until the next cron should execute.
        const auto timeout = server_.ComputeTimeout();

        if (timeout.count() <= 0) {
            // ProcessCron and process_backend must not run simultaneously
            auto lock = Lock{lock_};
            server_.ProcessCron();
        }

        Sleep(50ms);
    }
}

auto MessageProcessor::Imp::Start() noexcept -> void
{
    thread_ = std::thread(&Imp::run, this);
}

MessageProcessor::Imp::~Imp()
{
    cleanup();

    if (thread_.joinable()) { thread_.join(); }
}
}  // namespace opentxs::server

namespace opentxs::server
{
MessageProcessor::MessageProcessor(
    Server& server,
    const PasswordPrompt& reason) noexcept
    : imp_(std::make_unique<Imp>(server, reason).release())
{
    OT_ASSERT(imp_);
}

auto MessageProcessor::cleanup() noexcept -> void { imp_->cleanup(); }

auto MessageProcessor::DropIncoming(const int count) const noexcept -> void
{
    imp_->DropIncoming(count);
}

auto MessageProcessor::DropOutgoing(const int count) const noexcept -> void
{
    imp_->DropOutgoing(count);
}

auto MessageProcessor::init(
    const bool inproc,
    const int port,
    const Secret& privkey) noexcept(false) -> void
{
    imp_->init(inproc, port, privkey);
}

auto MessageProcessor::Start() noexcept -> void { imp_->Start(); }

MessageProcessor::~MessageProcessor()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::server
