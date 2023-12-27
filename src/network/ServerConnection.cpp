// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/ServerConnection.hpp"  // IWYU pragma: associated

#include <ServerReply.pb.h>
#include <ServerRequest.pb.h>
#include <atomic>
#include <chrono>
#include <compare>
#include <cstdint>
#include <memory>
#include <span>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <utility>

#include "internal/api/session/Endpoints.hpp"
#include "internal/core/Armored.hpp"
#include "internal/core/String.hpp"
#include "internal/core/contract/ServerContract.hpp"
#include "internal/network/ServerConnection.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/curve/Client.hpp"
#include "internal/network/zeromq/message/Message.hpp"
#include "internal/network/zeromq/socket/Dealer.hpp"
#include "internal/network/zeromq/socket/Publish.hpp"
#include "internal/network/zeromq/socket/Push.hpp"
#include "internal/network/zeromq/socket/Request.hpp"
#include "internal/network/zeromq/socket/Socket.hpp"
#include "internal/otx/common/Message.hpp"
#include "internal/otx/consensus/Server.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/serialization/protobuf/verify/ServerReply.hpp"
#include "internal/util/Flag.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/AddressType.hpp"  // IWYU pragma: keep
#include "opentxs/Types.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Factory.internal.hpp"
#include "opentxs/api/session/ZeroMQ.internal.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/Direction.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/otx/Reply.hpp"
#include "opentxs/otx/Request.hpp"
#include "opentxs/otx/ServerRequestType.hpp"  // IWYU pragma: keep
#include "opentxs/otx/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace zmq = opentxs::network::zeromq;

namespace opentxs::network
{
auto ServerConnection::Factory(
    const api::Session& api,
    const api::session::internal::ZeroMQ& zmq,
    const zeromq::socket::Publish& updates,
    const OTServerContract& contract) -> ServerConnection
{
    return {new ServerConnection::Imp(api, zmq, updates, contract)};
}

ServerConnection::ServerConnection(Imp* imp) noexcept
    : imp_(imp)
{
    assert_false(nullptr == imp);
}

ServerConnection::ServerConnection(ServerConnection&& rhs) noexcept
    : imp_{nullptr}
{
    swap(rhs);
}

auto ServerConnection::operator=(ServerConnection&& rhs) noexcept
    -> ServerConnection&
{
    swap(rhs);

    return *this;
}

ServerConnection::~ServerConnection()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}

auto ServerConnection::swap(ServerConnection& rhs) noexcept -> void
{
    std::swap(imp_, rhs.imp_);
}

auto ServerConnection::ChangeAddressType(const AddressType type) -> bool
{
    return imp_->ChangeAddressType(type);
}

auto ServerConnection::ClearProxy() -> bool { return imp_->ClearProxy(); }

auto ServerConnection::EnableProxy() -> bool { return imp_->EnableProxy(); }

auto ServerConnection::Send(
    const otx::context::Server& context,
    const otx::context::ServerPrivate& data,
    const Message& message,
    const PasswordPrompt& reason,
    const Push push) -> otx::client::NetworkReplyMessage
{
    return imp_->Send(context, data, message, reason, push);
}
auto ServerConnection::Status() const -> bool { return imp_->Status(); }

ServerConnection::Imp::Imp(
    const api::Session& api,
    const api::session::internal::ZeroMQ& zmq,
    const zeromq::socket::Publish& updates,
    const OTServerContract& contract)
    : zmq_(zmq)
    , api_(api)
    , updates_(updates)
    , server_id_(api_.Factory().NotaryIDFromBase58(
          contract->ID().asBase58(api_.Crypto())))
    , address_type_(zmq.DefaultAddressType())
    , remote_contract_(contract)
    , thread_()
    , callback_(zeromq::ListenCallback::Factory(
          [this](const auto& in) { process_incoming(in); }))
    , registration_socket_(zmq.Context().Internal().DealerSocket(
          callback_,
          zmq::socket::Direction::Connect,
          "ServerConnection registration"))
    , socket_(zmq.Context().Internal().RequestSocket())
    , notification_socket_(
          zmq.Context().Internal().PushSocket(zmq::socket::Direction::Connect))
    , last_activity_()
    , sockets_ready_(Flag::Factory(false))
    , status_(Flag::Factory(false))
    , use_proxy_(Flag::Factory(false))
    , registration_lock_()
    , registered_for_push_()
{
    thread_ = std::thread(&Imp::activity_timer, this);
    const auto started = notification_socket_->Start(
        api_.Endpoints().Internal().ProcessPushNotification().data());

    assert_true(started);
}

ServerConnection::Imp::~Imp()
{
    if (thread_.joinable()) { thread_.join(); }
}

auto ServerConnection::Imp::activity_timer() -> void
{
    while (zmq_.Running()) {
        const auto limit = zmq_.KeepAlive();
        const auto now = sClock::now();
        const auto last = last_activity_.load();
        const auto duration = now - last;

        if (duration > limit) {
            if (limit > 0s) {
                const auto result = socket_->Send(zeromq::Message{});

                if (otx::client::SendResult::TIMEOUT != result.first) {
                    reset_timer();

                    if (status_->On()) { publish(); }
                }
            } else {
                if (status_->Off()) { publish(); }
            }
        }

        sleep(1s);
    }
}

auto ServerConnection::Imp::async_socket(const Lock& lock) const
    -> OTZMQDealerSocket
{
    auto output = zmq_.Context().Internal().DealerSocket(
        callback_, zmq::socket::Direction::Connect, "ServerConnection async");
    set_proxy(lock, output);
    set_timeouts(lock, output);
    set_curve(lock, output);
    output->Start(endpoint());

    return output;
}

auto ServerConnection::Imp::ChangeAddressType(const AddressType type) -> bool
{
    auto lock = Lock{lock_};
    address_type_ = type;
    reset_socket(lock);

    return true;
}

auto ServerConnection::Imp::ClearProxy() -> bool
{
    auto lock = Lock{lock_};
    use_proxy_->Off();
    reset_socket(lock);

    return true;
}

auto ServerConnection::Imp::EnableProxy() -> bool
{
    auto lock = Lock{lock_};
    use_proxy_->On();
    reset_socket(lock);

    return true;
}

auto ServerConnection::Imp::disable_push(const identifier::Nym& nymID) -> void
{
    const auto registrationLock = Lock{registration_lock_};
    registered_for_push_[nymID] = true;
}

auto ServerConnection::Imp::endpoint() const -> UnallocatedCString
{
    std::uint32_t port{0};
    UnallocatedCString hostname{""};
    AddressType type{};
    const auto have =
        remote_contract_->ConnectInfo(hostname, port, type, address_type_);

    if (false == have) {
        LogError()()("Failed retrieving connection info from server contract.")
            .Flush();

        LogAbort()().Abort();
    }

    const auto endpoint = form_endpoint(type, hostname, port);
    LogError()("Establishing connection to: ")(endpoint).Flush();

    return endpoint;
}

auto ServerConnection::Imp::form_endpoint(
    AddressType type,
    UnallocatedCString hostname,
    std::uint32_t port) const -> UnallocatedCString
{
    auto output = std::stringstream{};

    if (AddressType::Inproc == type) {
        output << hostname;
    } else {
        output << "tcp://";
        output << hostname;
    }

    output << ":";
    output << std::to_string(port);

    return output.str();
}

auto ServerConnection::Imp::get_async(const Lock& lock)
    -> zeromq::socket::Dealer&
{
    assert_true(verify_lock(lock));

    if (false == sockets_ready_.get()) {
        registration_socket_ = async_socket(lock);
        socket_ = sync_socket(lock);
        sockets_ready_->On();
    }

    return registration_socket_;
}

auto ServerConnection::Imp::get_sync(const Lock& lock)
    -> zeromq::socket::Request&
{
    assert_true(verify_lock(lock));

    if (false == sockets_ready_.get()) {
        registration_socket_ = async_socket(lock);
        socket_ = sync_socket(lock);
        sockets_ready_->On();
    }

    return socket_;
}

auto ServerConnection::Imp::get_timeout() -> Time
{
    return Clock::now() + zmq_.SendTimeout();
}

auto ServerConnection::Imp::process_incoming(const zeromq::Message& in) -> void
{
    if (status_->On()) { publish(); }

    try {
        const auto body = in.Payload();
        const auto& payload = [&]() -> auto& {
            if (2u > body.size()) {
                throw std::runtime_error{"payload missing"};
            }

            if ((2u == body.size()) && (0u == body[1].size())) {

                return body[0];
            }

            const auto type = [&] {
                try {

                    return body[0].as<WorkType>();
                } catch (...) {
                    throw std::runtime_error{"unknown message type"};
                }
            }();

            switch (type) {
                case WorkType::OTXPush: {

                    return body[1];
                }
                case WorkType::OTXResponse:
                default: {
                    throw std::runtime_error{"unsupported message type"};
                }
            }
        }();
        const auto proto = proto::Factory<proto::ServerReply>(payload);

        if (false == proto::Validate(proto, VERBOSE)) {
            throw std::runtime_error{"invalid serialization"};
        }

        const auto message = otx::Reply::Factory(api_, proto);

        if (false == message.Validate()) {
            throw std::runtime_error{"Invalid message"};
        }

        notification_socket_->Send([&] {
            auto out = zeromq::Message{};
            out.AddFrame(payload);  // TODO std::move

            return out;
        }());
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();
    }
}

auto ServerConnection::Imp::publish() const -> void
{
    updates_.Send([&] {
        const auto status = bool{status_.get()};
        auto work = network::zeromq::tagged_message(
            WorkType::OTXConnectionStatus, true);
        work.AddFrame(server_id_);
        work.AddFrame(status);

        return work;
    }());
}

auto ServerConnection::Imp::register_for_push(
    const otx::context::Server& context,
    const otx::context::ServerPrivate& data,
    const PasswordPrompt& reason) -> void
{
    if (2 > context.Request(data)) {
        LogVerbose()()("Nym is not yet registered").Flush();

        return;
    }

    const auto registrationLock = Lock{registration_lock_};
    const auto& nymID = context.Signer()->ID();
    auto& isRegistered = registered_for_push_[nymID];

    if (isRegistered) { return; }

    auto request = otx::Request::Factory(
        api_,
        context.Signer(),
        context.Notary(),
        otx::ServerRequestType::Activate,
        0,
        reason);
    request.SetIncludeNym(true, reason);
    auto message = zmq::Message{};
    message.AddFrame();
    auto serialized = proto::ServerRequest{};
    if (false == request.Serialize(serialized)) {
        LogVerbose()()("Failed to serialize request.").Flush();

        return;
    }
    message.Internal().AddFrame(serialized);
    message.AddFrame();
    const auto socketLock = Lock{lock_};
    isRegistered = get_async(socketLock).Send(std::move(message));
}

auto ServerConnection::Imp::reset_socket(const Lock& lock) -> void
{
    assert_true(verify_lock(lock));

    sockets_ready_->Off();
}

auto ServerConnection::Imp::reset_timer() -> void
{
    last_activity_.store(sTime{});
}

auto ServerConnection::Imp::Send(
    const otx::context::Server& context,
    const otx::context::ServerPrivate& data,
    const Message& message,
    const PasswordPrompt& reason,
    const Push push) -> otx::client::NetworkReplyMessage
{
    struct Cleanup {
        const Lock& lock_;
        Imp& connection_;
        otx::client::SendResult& result_;
        std::shared_ptr<Message>& reply_;
        bool success_{false};

        void SetStatus(const otx::client::SendResult status)
        {
            if (otx::client::SendResult::VALID_REPLY == status) {
                success_ = true;
            }
        }

        Cleanup(
            const Lock& lock,
            Imp& connection,
            otx::client::SendResult& result,
            std::shared_ptr<Message>& reply)
            : lock_(lock)
            , connection_(connection)
            , result_(result)
            , reply_(reply)
        {
        }

        ~Cleanup()
        {
            if (false == success_) {
                connection_.reset_socket(lock_);
                reply_.reset();
            }
        }
    };

    if (Push::Enable == push) {
        LogTrace()()("Registering for push").Flush();
        register_for_push(context, data, reason);
    } else {
        LogTrace()()("Skipping push").Flush();
        disable_push(context.Signer()->ID());
    }

    otx::client::NetworkReplyMessage output{
        otx::client::SendResult::Error, nullptr};
    auto& status = output.first;
    auto& reply = output.second;
    reply.reset(api_.Factory().Internal().Session().Message().release());

    assert_true(false != bool(reply));

    auto raw = String::Factory();
    message.SaveContractRaw(raw);
    auto envelope = Armored::Factory(api_.Crypto(), raw);

    if (false == envelope->Exists()) {
        LogError()()("Failed to armor message").Flush();

        return output;
    }

    const auto socketLock = Lock{lock_};
    Cleanup cleanup(socketLock, *this, status, reply);
    auto sendresult = get_sync(socketLock).Send([&] {
        auto out = zeromq::Message{};
        out.AddFrame(envelope->Get());

        return out;
    }());

    if (status_->On()) { publish(); }

    status = sendresult.first;
    auto in = sendresult.second;
    auto replymessage{api_.Factory().Internal().Session().Message()};

    assert_true(false != bool(replymessage));

    if (otx::client::SendResult::TIMEOUT == status) {
        LogError()()("Reply timeout.").Flush();
        cleanup.SetStatus(otx::client::SendResult::TIMEOUT);

        return output;
    }

    try {
        const auto body = in.Payload();
        const auto& payload = [&] {
            if (0u == body.size()) {
                throw std::runtime_error{"Empty reply"};
            } else if (1u == body.size()) {

                return body[0];
            } else if (0u == body[1].size()) {
                throw std::runtime_error{
                    "Push notification received as a reply"};
            } else {
                const auto type = [&] {
                    try {

                        return body[0].as<WorkType>();
                    } catch (...) {
                        throw std::runtime_error{"Unknown message type"};
                    }
                }();

                switch (type) {
                    case WorkType::OTXLegacyXML: {

                        return body[1];
                    }
                    default: {
                        throw std::runtime_error{"Unsupported message type"};
                    }
                }
            }
        }();

        if (0 == payload.size()) {
            throw std::runtime_error{"Invalid reply message"};
        }

        const auto serialized = [&] {
            const auto armored = [&] {
                auto out = Armored::Factory(api_.Crypto());
                out->Set(UnallocatedCString{payload.Bytes()}.c_str());

                return out;
            }();
            auto out = String::Factory();
            armored->GetString(out);

            return out;
        }();
        const auto loaded = replymessage->LoadContractFromString(serialized);

        if (loaded) {
            reply = std::move(replymessage);
        } else {
            LogError()()(
                "Received server reply, but unable to instantiate it as a "
                "Message.")
                .Flush();
            cleanup.SetStatus(otx::client::SendResult::INVALID_REPLY);
        }

        cleanup.SetStatus(otx::client::SendResult::VALID_REPLY);
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();
        cleanup.SetStatus(otx::client::SendResult::INVALID_REPLY);
    }

    return output;
}

auto ServerConnection::Imp::set_curve(
    const Lock& lock,
    zeromq::curve::Client& socket) const -> void
{
    assert_true(verify_lock(lock));

    const auto set = socket.SetServerPubkey(remote_contract_);

    assert_true(set);
}

auto ServerConnection::Imp::set_proxy(
    const Lock& lock,
    zeromq::socket::Dealer& socket) const -> void
{
    assert_true(verify_lock(lock));

    if (false == use_proxy_.get()) { return; }

    auto proxy = zmq_.SocksProxy();

    if (false == proxy.empty()) {
        LogError()()("Setting proxy to ")(proxy).Flush();
        const auto set = socket.SetSocksProxy(proxy);

        assert_true(set);
    }
}

auto ServerConnection::Imp::set_timeouts(
    const Lock& lock,
    zeromq::socket::Socket& socket) const -> void
{
    assert_true(verify_lock(lock));

    const auto set = socket.SetTimeouts(
        zmq_.Linger(), zmq_.SendTimeout(), zmq_.ReceiveTimeout());

    assert_true(set);
}

auto ServerConnection::Imp::sync_socket(const Lock& lock) const
    -> OTZMQRequestSocket
{
    auto output = zmq_.Context().Internal().RequestSocket();
    set_timeouts(lock, output);
    set_curve(lock, output);
    output->Start(endpoint());

    return output;
}

auto ServerConnection::Imp::Status() const -> bool { return status_.get(); }
}  // namespace opentxs::network
