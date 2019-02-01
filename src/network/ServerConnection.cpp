// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/consensus/ServerContext.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/DealerSocket.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/PublishSocket.hpp"
#include "opentxs/network/zeromq/PushSocket.hpp"
#include "opentxs/network/zeromq/RequestSocket.hpp"
#include "opentxs/network/ServerConnection.hpp"
#include "opentxs/otx/Reply.hpp"
#include "opentxs/otx/Request.hpp"
#include "opentxs/Proto.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <map>
#include <memory>
#include <mutex>
#include <thread>

#include "ServerConnection.hpp"

namespace zmq = opentxs::network::zeromq;

template class opentxs::Pimpl<opentxs::network::ServerConnection>;
template class opentxs::Pimpl<zmq::RequestSocket>;

#define OT_METHOD "opentxs::ServerConnection::"

namespace opentxs::network
{
OTServerConnection ServerConnection::Factory(
    const api::Core& api,
    const api::network::ZMQ& zmq,
    const zeromq::PublishSocket& updates,
    const std::shared_ptr<const ServerContract>& contract)
{
    OT_ASSERT(contract)

    return OTServerConnection(
        new implementation::ServerConnection(api, zmq, updates, contract));
}
}  // namespace opentxs::network

namespace opentxs::network::implementation
{
ServerConnection::ServerConnection(
    const api::Core& api,
    const api::network::ZMQ& zmq,
    const zeromq::PublishSocket& updates,
    const std::shared_ptr<const ServerContract>& contract)
    : zmq_(zmq)
    , api_(api)
    , updates_(updates)
    , server_id_(contract->ID()->str())
    , address_type_(zmq.DefaultAddressType())
    , remote_contract_(contract)
    , thread_()
    , callback_(zeromq::ListenCallback::Factory(
          [=](const zeromq::Message& in) -> void {
              this->process_incoming(in);
          }))
    , registration_socket_(zmq.Context().DealerSocket(
          callback_,
          zmq::Socket::Direction::Connect))
    , socket_(zmq.Context().RequestSocket())
    , notification_socket_(
          zmq.Context().PushSocket(zmq::Socket::Direction::Connect))
    , last_activity_(std::time(nullptr))
    , sockets_ready_(Flag::Factory(false))
    , status_(Flag::Factory(false))
    , use_proxy_(Flag::Factory(false))
    , registration_lock_()
    , registered_for_push_()
{
    OT_ASSERT(remote_contract_)

    thread_ = std::thread(&ServerConnection::activity_timer, this);
    const auto started = notification_socket_->Start(
        api_.Endpoints().InternalProcessPushNotification());

    OT_ASSERT(started);
}

void ServerConnection::activity_timer()
{
    while (zmq_.Running()) {
        const auto limit = zmq_.KeepAlive();
        const auto now = std::chrono::seconds(std::time(nullptr));
        const auto last = std::chrono::seconds(last_activity_.load());
        const auto duration = now - last;

        if (duration > limit) {
            if (limit > std::chrono::seconds(0)) {
                const auto result = socket_->SendRequest(std::string(""));

                if (SendResult::TIMEOUT != result.first) {
                    reset_timer();

                    if (status_->On()) { publish(); };
                }
            } else {
                if (status_->Off()) { publish(); };
            }
        }

        Log::Sleep(std::chrono::seconds(1));
    }
}

OTZMQDealerSocket ServerConnection::async_socket(const Lock& lock) const
{
    auto output =
        zmq_.Context().DealerSocket(callback_, zmq::Socket::Direction::Connect);
    set_proxy(lock, output);
    set_timeouts(lock, output);
    set_curve(lock, output);
    output->Start(endpoint());

    return output;
}

bool ServerConnection::ChangeAddressType(const proto::AddressType type)
{
    Lock lock(lock_);
    address_type_ = type;
    reset_socket(lock);

    return true;
}

std::pair<bool, proto::ServerReply> ServerConnection::check_for_protobuf(
    const zeromq::Frame& frame)
{
    const auto candidate = Data::Factory(frame.data(), frame.size());
    std::pair<bool, proto::ServerReply> output{false, {}};
    auto& [valid, serialized] = output;
    serialized = proto::DataToProto<proto::ServerReply>(candidate);
    valid = proto::Validate(serialized, VERBOSE);

    return output;
}

bool ServerConnection::ClearProxy()
{
    Lock lock(lock_);
    use_proxy_->Off();
    reset_socket(lock);

    return true;
}

bool ServerConnection::EnableProxy()
{
    Lock lock(lock_);
    use_proxy_->On();
    reset_socket(lock);

    return true;
}

void ServerConnection::disable_push(const Identifier& nymID)
{
    Lock registrationLock(registration_lock_);
    registered_for_push_[nymID] = true;
}

std::string ServerConnection::endpoint() const
{
    std::uint32_t port{0};
    std::string hostname{""};
    proto::AddressType type{};
    const auto have =
        remote_contract_->ConnectInfo(hostname, port, type, address_type_);

    if (false == have) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed retrieving connection info from server contract.")
            .Flush();

        OT_FAIL;
    }

    const auto endpoint = form_endpoint(type, hostname, port);
    LogOutput("Establishing connection to: ")(endpoint).Flush();

    return endpoint;
}

std::string ServerConnection::form_endpoint(
    proto::AddressType type,
    std::string hostname,
    std::uint32_t port) const
{
    std::string output{};

    if (proto::ADDRESSTYPE_INPROC == type) {
        output += "inproc://opentxs/notary/";
        output += hostname;
        output += ":";
        output += std::to_string(port);
    } else {
        output += "tcp://";
        output += hostname;
        output += ":";
        output += std::to_string(port);
    }

    return output;
}

zeromq::DealerSocket& ServerConnection::get_async(const Lock& lock)
{
    OT_ASSERT(verify_lock(lock))

    if (false == sockets_ready_.get()) {
        registration_socket_ = async_socket(lock);
        socket_ = sync_socket(lock);
        sockets_ready_->On();
    }

    return registration_socket_;
}

zeromq::RequestSocket& ServerConnection::get_sync(const Lock& lock)
{
    OT_ASSERT(verify_lock(lock))

    if (false == sockets_ready_.get()) {
        registration_socket_ = async_socket(lock);
        socket_ = sync_socket(lock);
        sockets_ready_->On();
    }

    return socket_;
}

std::chrono::time_point<std::chrono::system_clock> ServerConnection::
    get_timeout()
{
    return std::chrono::system_clock::now() + zmq_.SendTimeout();
}

void ServerConnection::process_incoming(const proto::ServerReply& in)
{
    auto message = otx::Reply::Factory(api_, in);

    if (false == message->Validate()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid push notification.")
            .Flush();

        return;
    }

    notification_socket_->Push(proto::ProtoAsData(message->Contract()));
}

void ServerConnection::process_incoming(const zeromq::Message& in)
{
    if (status_->On()) { publish(); }

    auto message{api_.Factory().Message()};

    OT_ASSERT(false != bool(message));

    if (1 > in.Body().size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Received legacy reply on async socket.")
            .Flush();

        return;
    }

    auto& frame = *in.Body().begin();

    if (0 == frame.size()) { return; }

    if (1 < in.Body().size()) {
        const auto [isProto, reply] = check_for_protobuf(frame);

        if (isProto) {
            process_incoming(reply);

            return;
        }

        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Message should be a protobuf but isn't.")
            .Flush();

        return;
    }
}

void ServerConnection::publish() const
{
    const bool state(status_.get());
    auto message = zmq::Message::Factory();
    message->AddFrame(server_id_);
    message->AddFrame(Data::Factory(&state, sizeof(state)));
    updates_.Publish(message);
}

void ServerConnection::register_for_push(const ServerContext& context)
{
    if (2 > context.Request()) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Nym is not yet registered")
            .Flush();

        return;
    }

    Lock registrationLock(registration_lock_);
    const auto& nymID = context.Nym()->ID();
    auto& isRegistered = registered_for_push_[nymID];

    if (isRegistered) { return; }

    auto request = otx::Request::Factory(
        context.Nym(), context.Server(), proto::SERVERREQUEST_ACTIVATE);
    request->SetIncludeNym(true);
    auto message = zmq::Message::Factory();
    message->AddFrame();
    message->AddFrame(proto::ProtoAsData(request->Contract()));
    message->AddFrame();
    Lock socketLock(lock_);
    isRegistered = get_async(socketLock).Send(message);
}

void ServerConnection::reset_socket(const Lock& lock)
{
    OT_ASSERT(verify_lock(lock))

    sockets_ready_->Off();
}

void ServerConnection::reset_timer()
{
    last_activity_.store(std::time(nullptr));
}

NetworkReplyMessage ServerConnection::Send(
    const ServerContext& context,
    const Message& message,
    const Push push)
{
    struct Cleanup {
        const Lock& lock_;
        ServerConnection& connection_;
        SendResult& result_;
        std::shared_ptr<Message>& reply_;
        bool success_{false};

        void SetStatus(const SendResult status)
        {
            if (SendResult::VALID_REPLY == status) { success_ = true; }
        }

        Cleanup(
            const Lock& lock,
            ServerConnection& connection,
            SendResult& result,
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
        LogTrace(OT_METHOD)(__FUNCTION__)(": Registering for push").Flush();
        register_for_push(context);
    } else {
        LogTrace(OT_METHOD)(__FUNCTION__)(": Skipping push").Flush();
        disable_push(context.Nym()->ID());
    }

    NetworkReplyMessage output{SendResult::ERROR, nullptr};
    auto& status = output.first;
    auto& reply = output.second;
    reply.reset(api_.Factory().Message().release());

    OT_ASSERT(false != bool(reply));

    auto raw = String::Factory();
    message.SaveContractRaw(raw);
    auto envelope = Armored::Factory(raw);

    if (false == envelope->Exists()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to armor message").Flush();

        return output;
    }

    Lock socketLock(lock_);
    Cleanup cleanup(socketLock, *this, status, reply);
    auto request = zmq::Message::Factory(std::string(envelope->Get()));
    auto sendresult = get_sync(socketLock).SendRequest(request);

    if (status_->On()) { publish(); }

    status = sendresult.first;
    auto in = sendresult.second;
    auto replymessage{api_.Factory().Message()};

    OT_ASSERT(false != bool(replymessage));

    if (SendResult::TIMEOUT == status) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Reply timeout.").Flush();
        cleanup.SetStatus(SendResult::TIMEOUT);

        return output;
    }

    if (1 > in->Body().size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Empty reply.").Flush();
        cleanup.SetStatus(SendResult::INVALID_REPLY);

        return output;
    }

    if (1 < in->Body().size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Push notification received as a reply.")
            .Flush();
        cleanup.SetStatus(SendResult::INVALID_REPLY);

        return output;
    }

    auto& frame = *in->Body().begin();

    if (0 == frame.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid reply message.").Flush();
        cleanup.SetStatus(SendResult::INVALID_REPLY);

        return output;
    }

    auto armored = Armored::Factory();
    armored->Set(std::string(frame).c_str());
    auto serialized = String::Factory();
    armored->GetString(serialized);
    const auto loaded = replymessage->LoadContractFromString(serialized);

    if (loaded) {
        reply.reset(replymessage.release());
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Received server reply, but unable to instantiate it as a "
            "Message.")
            .Flush();
        cleanup.SetStatus(SendResult::INVALID_REPLY);
    }

    cleanup.SetStatus(SendResult::VALID_REPLY);

    return output;
}

void ServerConnection::set_curve(const Lock& lock, zeromq::CurveClient& socket)
    const
{
    OT_ASSERT(verify_lock(lock));

    const auto set = socket.SetServerPubkey(*remote_contract_);

    OT_ASSERT(set);
}

void ServerConnection::set_proxy(const Lock& lock, zeromq::DealerSocket& socket)
    const
{
    OT_ASSERT(verify_lock(lock));

    if (false == use_proxy_.get()) { return; }

    auto proxy = zmq_.SocksProxy();

    if (false == proxy.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Setting proxy to ")(proxy)
            .Flush();
        const auto set = socket.SetSocksProxy(proxy);

        OT_ASSERT(set);
    }
}

void ServerConnection::set_timeouts(const Lock& lock, zeromq::Socket& socket)
    const
{
    OT_ASSERT(verify_lock(lock));

    const auto set = socket.SetTimeouts(
        zmq_.Linger(), zmq_.SendTimeout(), zmq_.ReceiveTimeout());

    OT_ASSERT(set);
}

OTZMQRequestSocket ServerConnection::sync_socket(const Lock& lock) const
{
    auto output = zmq_.Context().RequestSocket();
    set_timeouts(lock, output);
    set_curve(lock, output);
    output->Start(endpoint());

    return output;
}

bool ServerConnection::Status() const { return status_.get(); }

ServerConnection::~ServerConnection()
{
    if (thread_.joinable()) { thread_.join(); }
}
}  // namespace opentxs::network::implementation
