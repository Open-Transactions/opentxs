// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/zeromq/socket/Raw.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>
#include <zmq.h>
#include <array>
#include <atomic>
#include <cstdint>
#include <exception>
#include <iostream>
#include <limits>
#include <span>
#include <utility>

#include "internal/network/zeromq/socket/Factory.hpp"
#include "internal/network/zeromq/socket/SocketType.hpp"
#include "internal/network/zeromq/socket/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
static auto zmq_socket_counter() noexcept -> std::atomic<std::ptrdiff_t>&
{
    static auto counter = std::atomic<std::ptrdiff_t>{0};

    return counter;
}

auto print(const network::zeromq::socket::Type type) noexcept -> const char*
{
    using enum network::zeromq::socket::Type;

    static constexpr auto map =
        frozen::make_unordered_map<network::zeromq::socket::Type, const char*>({
            {Request, "ZMQ_REQ"},
            {Reply, "ZMQ_REP"},
            {Publish, "ZMQ_PUB"},
            {Subscribe, "ZMQ_SUB"},
            {Pull, "ZMQ_PULL"},
            {Push, "ZMQ_PUSH"},
            {Pair, "ZMQ_PAIR"},
            {Dealer, "ZMQ_DEALER"},
            {Router, "ZMQ_ROUTER"},
        });

    try {

        return map.at(type);
    } catch (...) {

        return "";
    }
}

auto to_native(const network::zeromq::socket::Type type) noexcept -> int
{
    using enum network::zeromq::socket::Type;

    static const auto map =
        frozen::make_unordered_map<network::zeromq::socket::Type, int>({
            {Request, ZMQ_REQ},
            {Reply, ZMQ_REP},
            {Publish, ZMQ_PUB},
            {Subscribe, ZMQ_SUB},
            {Pull, ZMQ_PULL},
            {Push, ZMQ_PUSH},
            {Pair, ZMQ_PAIR},
            {Dealer, ZMQ_DEALER},
            {Router, ZMQ_ROUTER},
        });

    try {

        return map.at(type);
    } catch (...) {

        return 0;
    }
}

auto zmq_close_wrapper(void* socket) noexcept -> int
{
    const auto out = ::zmq_close(socket);
    --zmq_socket_counter();
    // TODO add optional logging

    return out;
}

auto zmq_socket_wrapper(void* context, int type) noexcept -> void*
{
    auto* const out = ::zmq_socket(context, type);
    ++zmq_socket_counter();
    // TODO add optional logging

    return out;
}
}  // namespace opentxs

namespace opentxs::factory
{
auto ZMQSocket(
    const network::zeromq::Context& context,
    const network::zeromq::socket::Type type) noexcept
    -> network::zeromq::socket::Raw
{
    using Imp = network::zeromq::socket::implementation::Raw;

    return std::make_unique<Imp>(context, type).release();
}

auto ZMQSocketNull() noexcept -> network::zeromq::socket::Raw
{
    using ReturnType = network::zeromq::socket::Raw;

    return std::make_unique<ReturnType::Imp>().release();
}
}  // namespace opentxs::factory

namespace opentxs::network::zeromq::socket::implementation
{
Raw::Raw(const Context& context, const socket::Type type) noexcept
    : Imp()
    , type_(type)
    , socket_(zmq_socket_wrapper(context, to_native(type_)), zmq_close_wrapper)
    , bound_endpoints_()
    , connected_endpoints_()
{
    SetLinger(0);
    SetIncomingHWM(default_hwm_);
    SetOutgoingHWM(default_hwm_);
    SetSendTimeout(default_send_timeout_);
}

auto Raw::Bind(const char* endpoint) noexcept -> bool
{
    if (0 != ::zmq_bind(Native(), endpoint)) {
        std::cerr << (OT_PRETTY_CLASS()) << "Failed to bind to (" << endpoint
                  << "): " << ::zmq_strerror(zmq_errno()) << std::endl;

        return false;
    } else {
        record_endpoint(bound_endpoints_);

        return true;
    }
}

auto Raw::ClearSubscriptions() noexcept -> bool
{
    if (0 != ::zmq_setsockopt(Native(), ZMQ_SUBSCRIBE, "", 0)) {
        std::cerr << "Failed to set ZMQ_SUBSCRIBE\n";
        std::cerr << ::zmq_strerror(zmq_errno()) << '\n';

        return false;
    } else {

        return true;
    }
}

auto Raw::Close() noexcept -> void
{
    Stop();
    socket_.reset();
}

auto Raw::Connect(const char* endpoint) noexcept -> bool
{
    if (0 != ::zmq_connect(Native(), endpoint)) {
        std::cerr << (OT_PRETTY_CLASS()) << "Failed to connect to (" << endpoint
                  << "): " << ::zmq_strerror(zmq_errno()) << std::endl;

        return false;
    } else {
        record_endpoint(connected_endpoints_);

        return true;
    }
}

auto Raw::Disconnect(const char* endpoint) noexcept -> bool
{
    const auto rc = ::zmq_disconnect(Native(), endpoint);

    if (0 != rc) {
        std::cerr << (OT_PRETTY_CLASS()) << ::zmq_strerror(zmq_errno())
                  << std::endl;
    }

    connected_endpoints_.erase(endpoint);

    return 0 == rc;
}

auto Raw::DisconnectAll() noexcept -> bool
{
    for (const auto& endpoint : connected_endpoints_) {
        ::zmq_disconnect(Native(), endpoint.c_str());
    }

    connected_endpoints_.clear();

    return true;
}

auto Raw::EnableCurveClient(
    const ReadView serverKey,
    const ReadView publicKey,
    const ReadView secretKey) noexcept -> bool
{
    constexpr int server{0};
    auto rc =
        ::zmq_setsockopt(Native(), ZMQ_CURVE_SERVER, &server, sizeof(server));

    if (0 != rc) {
        std::cerr << (OT_PRETTY_CLASS()) << "Failed to set ZMQ_CURVE_SERVER"
                  << ": " << ::zmq_strerror(zmq_errno()) << std::endl;

        return false;
    }

    rc = ::zmq_setsockopt(
        Native(), ZMQ_CURVE_SERVERKEY, serverKey.data(), serverKey.size());

    if (0 != rc) {
        std::cerr << (OT_PRETTY_CLASS()) << "Failed to set server key"
                  << ": " << ::zmq_strerror(zmq_errno()) << std::endl;

        return false;
    }

    rc = ::zmq_setsockopt(
        Native(), ZMQ_CURVE_PUBLICKEY, publicKey.data(), publicKey.size());

    if (0 != rc) {
        std::cerr << (OT_PRETTY_CLASS()) << "Failed to set public key"
                  << ": " << ::zmq_strerror(zmq_errno()) << std::endl;

        return false;
    }

    rc = ::zmq_setsockopt(
        Native(), ZMQ_CURVE_SECRETKEY, secretKey.data(), secretKey.size());

    if (0 != rc) {
        std::cerr << (OT_PRETTY_CLASS()) << "Failed to set secret key"
                  << ": " << ::zmq_strerror(zmq_errno()) << std::endl;

        return false;
    }

    return true;
}

auto Raw::EnableCurveServer(const ReadView secretKey) noexcept -> bool
{
    return SetPrivateKey(secretKey);
}

auto Raw::record_endpoint(Endpoints& out) noexcept -> void
{
    auto buffer = std::array<char, 256>{};
    auto bytes = buffer.size();
    const auto rc =
        ::zmq_getsockopt(Native(), ZMQ_LAST_ENDPOINT, buffer.data(), &bytes);

    if (0 != rc) { std::terminate(); }

    out.emplace(buffer.data(), bytes);
}

auto Raw::Send(Message&& msg, const char* file, int line, bool silent) noexcept
    -> bool
{
    const auto sent = send(std::move(msg), ZMQ_DONTWAIT, file, line, silent);

    OT_ASSERT(sent);

    return sent;
}

auto Raw::SendDeferred(
    Message&& msg,
    const char* file,
    int line,
    bool silent) noexcept -> bool
{
    return send(std::move(msg), 0, file, line, silent);
}

auto Raw::SendExternal(
    Message&& msg,
    const char* file,
    int line,
    bool silent) noexcept -> bool
{
    return send(std::move(msg), ZMQ_DONTWAIT, file, line, silent);
}

auto Raw::send(
    Message&& msg,
    const int baseFlags,
    const char* file,
    int line,
    bool silent) noexcept -> bool
{
    auto sent{true};
    auto frames = msg.get();
    const auto parts = frames.size();
    auto counter = 0_uz;

    for (auto& frame : frames) {
        auto flags{baseFlags};

        if (++counter < parts) { flags |= ZMQ_SNDMORE; }

        sent &= (-1 != ::zmq_msg_send(frame, Native(), flags));
    }

    if ((false == sent) && (false == silent)) {
        std::cerr << (OT_PRETTY_CLASS()) << "Send error from " << file << ": "
                  << std::to_string(line) << ": " << ::zmq_strerror(zmq_errno())
                  << '\n'
                  << PrintStackTrace();
    }

    return sent;
}

auto Raw::SetExposedUntrusted() noexcept -> bool
{
    auto output = SetIncomingHWM(untrusted_hwm_);
    output &= SetOutgoingHWM(untrusted_hwm_);
    output &= SetMaxMessageSize(untrusted_max_message_size_);

    return output;
}

auto Raw::SetIncomingHWM(int value) noexcept -> bool
{
    const auto rc =
        ::zmq_setsockopt(Native(), ZMQ_RCVHWM, &value, sizeof(value));

    if (0 != rc) {
        std::cerr << (OT_PRETTY_CLASS()) << "Failed to set ZMQ_RCVHWM\n";
        std::cerr << ::zmq_strerror(zmq_errno()) << '\n';

        return false;
    }

    return true;
}

auto Raw::SetLinger(int value) noexcept -> bool
{
    const auto rc =
        ::zmq_setsockopt(Native(), ZMQ_LINGER, &value, sizeof(value));

    if (0 != rc) {
        std::cerr << (OT_PRETTY_CLASS()) << "Failed to set ZMQ_LINGER\n";
        std::cerr << ::zmq_strerror(zmq_errno()) << '\n';

        return false;
    }

    return true;
}

auto Raw::SetMonitor(const char* endpoint, int events) noexcept -> bool
{
    const auto rc = ::zmq_socket_monitor(Native(), endpoint, events);

    if (0 != rc) {
        std::cerr << (OT_PRETTY_CLASS()) << "Failed zmq_socket_monitor\n";
        std::cerr << ::zmq_strerror(zmq_errno()) << '\n';

        return false;
    }

    return true;
}

auto Raw::SetMaxMessageSize(std::size_t arg) noexcept -> bool
{
    using ZMQArg = std::int64_t;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtautological-constant-out-of-range-compare"
    if (std::numeric_limits<ZMQArg>::max() < arg) {
        std::cerr << (OT_PRETTY_CLASS()) << "Argument too large\n";

        return false;
    }
#pragma GCC diagnostic pop

    const auto rc =
        ::zmq_setsockopt(Native(), ZMQ_MAXMSGSIZE, &arg, sizeof(arg));

    if (0 != rc) {
        std::cerr << (OT_PRETTY_CLASS()) << "Failed to set ZMQ_MAXMSGSIZE\n";
        std::cerr << ::zmq_strerror(zmq_errno()) << '\n';

        return false;
    }

    return true;
}

auto Raw::SetOutgoingHWM(int value) noexcept -> bool
{
    const auto rc =
        ::zmq_setsockopt(Native(), ZMQ_SNDHWM, &value, sizeof(value));

    if (0 != rc) {
        std::cerr << (OT_PRETTY_CLASS()) << "Failed to set ZMQ_SNDHWM\n";
        std::cerr << ::zmq_strerror(zmq_errno()) << '\n';

        return false;
    }

    return true;
}

auto Raw::SetPrivateKey(ReadView key) noexcept -> bool
{
    if (CURVE_KEY_BYTES != key.size()) {
        std::cerr << (OT_PRETTY_CLASS()) << "Invalid private key" << std::endl;

        return false;
    }

    const int server{1};
    auto rc =
        ::zmq_setsockopt(Native(), ZMQ_CURVE_SERVER, &server, sizeof(server));

    if (0 != rc) {
        std::cerr << (OT_PRETTY_CLASS()) << "Failed to set ZMQ_CURVE_SERVER"
                  << std::endl;

        return false;
    }

    rc =
        ::zmq_setsockopt(Native(), ZMQ_CURVE_SECRETKEY, key.data(), key.size());

    if (0 != rc) {
        std::cerr << (OT_PRETTY_CLASS()) << "Failed to set private key"
                  << std::endl;

        return false;
    }

    return true;
}

auto Raw::SetRouterHandover(bool value) noexcept -> bool
{
    const auto data = value ? int{1} : int{0};
    const auto rc =
        ::zmq_setsockopt(Native(), ZMQ_ROUTER_HANDOVER, &data, sizeof(data));

    if (0 != rc) {
        std::cerr << (OT_PRETTY_CLASS())
                  << "Failed to set ZMQ_ROUTER_HANDOVER\n";
        std::cerr << ::zmq_strerror(zmq_errno()) << '\n';

        return false;
    }

    return true;
}

auto Raw::SetRoutingID(ReadView id) noexcept -> bool
{
    const auto set =
        ::zmq_setsockopt(Native(), ZMQ_ROUTING_ID, id.data(), id.size());

    if (0 != set) {
        std::cerr << (OT_PRETTY_CLASS()) << ::zmq_strerror(zmq_errno())
                  << std::endl;

        return false;
    }

    return true;
}

auto Raw::SetSendTimeout(std::chrono::milliseconds value) noexcept -> bool
{
    const auto ms = value.count();
    using ZMQArg = int;

    if (std::numeric_limits<ZMQArg>::max() < ms) {
        std::cerr << (OT_PRETTY_CLASS()) << "Argument too large\n";

        return false;
    }

    const auto arg = [&]() -> ZMQArg {
        if (0 > ms) {

            return -1;
        } else {
            return static_cast<ZMQArg>(ms);
        }
    }();
    const auto set =
        ::zmq_setsockopt(Native(), ZMQ_SNDTIMEO, &arg, sizeof(arg));

    if (0 != set) {
        std::cerr << (OT_PRETTY_CLASS()) << ::zmq_strerror(zmq_errno())
                  << std::endl;

        return false;
    }

    return true;
}

auto Raw::SetZAPDomain(ReadView domain) noexcept -> bool
{
    const auto set = ::zmq_setsockopt(
        Native(), ZMQ_ZAP_DOMAIN, domain.data(), domain.size());

    if (0 != set) {
        std::cerr << (OT_PRETTY_CLASS()) << ::zmq_strerror(zmq_errno())
                  << std::endl;

        return false;
    }

    return true;
}

auto Raw::Stop() noexcept -> void
{
    DisconnectAll();
    UnbindAll();
}

auto Raw::Unbind(const char* endpoint) noexcept -> bool
{
    const auto rc = ::zmq_unbind(Native(), endpoint);

    if (0 != rc) {
        std::cerr << (OT_PRETTY_CLASS()) << ::zmq_strerror(zmq_errno())
                  << std::endl;
    }

    connected_endpoints_.erase(endpoint);

    return 0 == rc;
}

auto Raw::UnbindAll() noexcept -> bool
{
    auto output{true};

    for (const auto& endpoint : bound_endpoints_) {
        const auto rc = ::zmq_unbind(Native(), endpoint.c_str());

        if (0 != rc) {
            std::cerr << (OT_PRETTY_CLASS()) << ::zmq_strerror(zmq_errno())
                      << std::endl;
            output = false;
        }
    }

    bound_endpoints_.clear();

    return output;
}

auto Raw::WaitForSend() noexcept -> bool { return wait(ZMQ_POLLOUT); }

auto Raw::wait(int flags) noexcept -> bool
{
    auto poll = ::zmq_pollitem_t{};
    poll.socket = Native();
    poll.events = flags;
    static constexpr auto timeout = 1000ms;
    const auto rc = ::zmq_poll(std::addressof(poll), 1, timeout.count());

    if (0 > rc) {
        std::cerr << (OT_PRETTY_CLASS()) << ::zmq_strerror(zmq_errno())
                  << std::endl;

        return false;
    } else {

        return 0 < rc;
    }
}

Raw::~Raw() { Close(); }
}  // namespace opentxs::network::zeromq::socket::implementation

namespace opentxs::network::zeromq::socket
{
auto swap(Raw& lhs, Raw& rhs) noexcept -> void { lhs.swap(rhs); }

Raw::Raw(Imp* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(imp);
}

Raw::Raw(Raw&& rhs) noexcept
    : Raw(std::make_unique<Imp>().release())
{
    swap(rhs);
}

auto Raw::operator=(Raw&& rhs) noexcept -> Raw&
{
    swap(rhs);

    return *this;
}

auto Raw::Bind(const char* endpoint) noexcept -> bool
{
    return imp_->Bind(endpoint);
}

auto Raw::ClearSubscriptions() noexcept -> bool
{
    return imp_->ClearSubscriptions();
}

auto Raw::Close() noexcept -> void { imp_->Close(); }

auto Raw::Connect(const char* endpoint) noexcept -> bool
{
    return imp_->Connect(endpoint);
}

auto Raw::Disconnect(const char* endpoint) noexcept -> bool
{
    return imp_->Disconnect(endpoint);
}

auto Raw::DisconnectAll() noexcept -> bool { return imp_->DisconnectAll(); }

auto Raw::EnableCurveClient(
    const ReadView serverKey,
    const ReadView publicKey,
    const ReadView secretKey) noexcept -> bool
{
    return imp_->EnableCurveClient(serverKey, publicKey, secretKey);
}

auto Raw::EnableCurveServer(const ReadView secretKey) noexcept -> bool
{
    return imp_->EnableCurveServer(secretKey);
}

auto Raw::ID() const noexcept -> SocketID { return imp_->ID(); }

auto Raw::Native() noexcept -> void* { return imp_->Native(); }

auto Raw::Send(Message&& msg, const char* file, int line, bool silent) noexcept
    -> bool
{
    return imp_->Send(std::move(msg), file, line, silent);
}

auto Raw::SendDeferred(
    Message&& msg,
    const char* file,
    int line,
    bool silent) noexcept -> bool
{
    return imp_->SendDeferred(std::move(msg), file, line, silent);
}

auto Raw::SendExternal(
    Message&& msg,
    const char* file,
    int line,
    bool silent) noexcept -> bool
{
    return imp_->SendExternal(std::move(msg), file, line, silent);
}

auto Raw::SetExposedUntrusted() noexcept -> bool
{
    return imp_->SetExposedUntrusted();
}

auto Raw::SetIncomingHWM(int value) noexcept -> bool
{
    return imp_->SetIncomingHWM(value);
}

auto Raw::SetLinger(int value) noexcept -> bool
{
    return imp_->SetLinger(value);
}

auto Raw::SetMonitor(const char* endpoint, int events) noexcept -> bool
{
    return imp_->SetMonitor(endpoint, events);
}

auto Raw::SetOutgoingHWM(int value) noexcept -> bool
{
    return imp_->SetOutgoingHWM(value);
}

auto Raw::SetPrivateKey(ReadView key) noexcept -> bool
{
    return imp_->SetPrivateKey(key);
}

auto Raw::SetRouterHandover(bool value) noexcept -> bool
{
    return imp_->SetRouterHandover(value);
}

auto Raw::SetRoutingID(ReadView id) noexcept -> bool
{
    return imp_->SetRoutingID(id);
}

auto Raw::SetSendTimeout(std::chrono::milliseconds value) noexcept -> bool
{
    return imp_->SetSendTimeout(value);
}

auto Raw::SetZAPDomain(ReadView domain) noexcept -> bool
{
    return imp_->SetZAPDomain(domain);
}

auto Raw::Stop() noexcept -> void { return imp_->Stop(); }

auto Raw::swap(Raw& rhs) noexcept -> void { std::swap(imp_, rhs.imp_); }

auto Raw::Type() const noexcept -> socket::Type { return imp_->Type(); }

auto Raw::Unbind(const char* endpoint) noexcept -> bool
{
    return imp_->Unbind(endpoint);
}

auto Raw::UnbindAll() noexcept -> bool { return imp_->UnbindAll(); }

auto Raw::WaitForSend() noexcept -> bool { return imp_->WaitForSend(); }

Raw::~Raw()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::network::zeromq::socket
