// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/session/ZeroMQPrivate.hpp"  // IWYU pragma: associated

#include <atomic>
#include <cstdint>
#include <utility>

#include "internal/core/String.hpp"
#include "internal/network/ServerConnection.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/socket/Publish.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/AddressType.hpp"  // IWYU pragma: keep
#include "opentxs/Types.hpp"
#include "opentxs/api/Network.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/api/Settings.internal.hpp"
#include "opentxs/api/network/ZeroMQ.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/api/session/Wallet.internal.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/identifier/Notary.hpp"        // IWYU pragma: keep
#include "opentxs/network/ConnectionState.hpp"  // IWYU pragma: keep
#include "opentxs/network/Types.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

#define CLIENT_SEND_TIMEOUT_SECONDS 5
#define CLIENT_RECV_TIMEOUT_SECONDS 5
#define CLIENT_SOCKET_LINGER_SECONDS 0
#define CLIENT_SEND_TIMEOUT CLIENT_SEND_TIMEOUT_SECONDS
#define CLIENT_RECV_TIMEOUT CLIENT_RECV_TIMEOUT_SECONDS
#define KEEP_ALIVE_SECONDS 30

namespace opentxs::api::session
{
ZeroMQPrivate::ZeroMQPrivate(
    const api::Session& api,
    const Flag& running) noexcept
    : api_(api)
    , running_(running)
    , linger_(std::chrono::seconds(CLIENT_SOCKET_LINGER_SECONDS))
    , receive_timeout_(std::chrono::seconds(CLIENT_RECV_TIMEOUT))
    , send_timeout_(std::chrono::seconds(CLIENT_SEND_TIMEOUT))
    , keep_alive_(0s)
    , lock_()
    , socks_proxy_()
    , server_connections_()
    , status_publisher_(
          api_.Network().ZeroMQ().Context().Internal().PublishSocket())
{
    // WARNING: do not access api_.Wallet() during construction
    status_publisher_->Start(api_.Endpoints().ConnectionStatus().data());

    const auto lock = Lock{lock_};

    init(lock);
}

auto ZeroMQPrivate::Context() const noexcept
    -> const opentxs::network::zeromq::Context&
{
    return api_.Network().ZeroMQ().Context();
}

auto ZeroMQPrivate::DefaultAddressType() const noexcept -> AddressType
{
    bool changed{false};
    const std::int64_t defaultType{
        static_cast<std::int64_t>(AddressType::IPV4)};
    std::int64_t configuredType{static_cast<std::int64_t>(AddressType::Error)};
    api_.Config().Internal().CheckSet_long(
        String::Factory("Connection"),
        String::Factory("preferred_address_type"),
        defaultType,
        configuredType,
        changed);

    if (changed && (false == api_.Config().Internal().Save())) {
        LogAbort()()("failed to save config file").Abort();
    }

    return static_cast<AddressType>(configuredType);
}

auto ZeroMQPrivate::init(const Lock& lock) const noexcept -> void
{
    assert_true(verify_lock(lock));

    const auto& config = api_.Config().Internal();
    bool notUsed{false};
    std::int64_t linger{0};
    config.CheckSet_long(
        String::Factory("latency"),
        String::Factory("linger"),
        CLIENT_SOCKET_LINGER_SECONDS,
        linger,
        notUsed);
    linger_.store(std::chrono::seconds(linger));
    std::int64_t send{0};
    config.CheckSet_long(
        String::Factory("latency"),
        String::Factory("send_timeout"),
        CLIENT_SEND_TIMEOUT,
        send,
        notUsed);
    send_timeout_.store(std::chrono::seconds(send));
    std::int64_t receive{0};
    config.CheckSet_long(
        String::Factory("latency"),
        String::Factory("recv_timeout"),
        CLIENT_RECV_TIMEOUT,
        receive,
        notUsed);
    receive_timeout_.store(std::chrono::seconds(receive));
    auto socks = String::Factory();
    bool haveSocksConfig{false};
    const bool configChecked = config.Check_str(
        String::Factory("Connection"),
        String::Factory("socks_proxy"),
        socks,
        haveSocksConfig);
    std::int64_t keepAlive{0};
    config.CheckSet_long(
        String::Factory("Connection"),
        String::Factory("keep_alive"),
        KEEP_ALIVE_SECONDS,
        keepAlive,
        notUsed);
    keep_alive_.store(std::chrono::seconds(keepAlive));

    if (configChecked && haveSocksConfig && socks->Exists()) {
        socks_proxy_ = socks->Get();
    }

    if (false == config.Save()) {
        LogAbort()()("failed to save config file").Abort();
    }
}

auto ZeroMQPrivate::KeepAlive() const noexcept -> std::chrono::seconds
{
    return keep_alive_.load();
}

auto ZeroMQPrivate::KeepAlive(
    const std::chrono::seconds duration) const noexcept -> void
{
    keep_alive_.store(duration);
}

auto ZeroMQPrivate::Linger() const noexcept -> std::chrono::seconds
{
    return linger_.load();
}

auto ZeroMQPrivate::ReceiveTimeout() const noexcept -> std::chrono::seconds
{
    return receive_timeout_.load();
}

auto ZeroMQPrivate::RefreshConfig() const noexcept -> void
{
    const auto lock = Lock{lock_};

    return init(lock);
}

auto ZeroMQPrivate::Running() const noexcept -> const Flag& { return running_; }

auto ZeroMQPrivate::SendTimeout() const noexcept -> std::chrono::seconds
{
    return send_timeout_.load();
}

auto ZeroMQPrivate::Server(const identifier::Notary& id) const noexcept(false)
    -> opentxs::network::ServerConnection&
{
    const auto lock = Lock{lock_};
    auto existing = server_connections_.find(id);

    if (server_connections_.end() != existing) { return existing->second; }

    auto contract = api_.Wallet().Internal().Server(id);
    auto [it, created] = server_connections_.emplace(
        id,
        opentxs::network::ServerConnection::Factory(
            api_, *this, status_publisher_, contract));
    auto& connection = it->second;

    assert_true(created);

    if (false == socks_proxy_.empty()) { connection.EnableProxy(); }

    return connection;
}

auto ZeroMQPrivate::SetSocksProxy(
    const UnallocatedCString& proxy) const noexcept -> bool
{
    bool notUsed{false};
    bool set = api_.Config().Internal().Set_str(
        String::Factory("Connection"),
        String::Factory("socks_proxy"),
        String::Factory(proxy),
        notUsed);

    if (false == set) {
        LogError()()("Unable to set socks proxy.").Flush();

        return false;
    }

    if (false == api_.Config().Internal().Save()) {
        LogError()()("Unable to set save config.").Flush();

        return false;
    }

    const auto lock = Lock{lock_};
    socks_proxy_ = proxy;

    for (auto& it : server_connections_) {
        opentxs::network::ServerConnection& connection = it.second;

        if (proxy.empty()) {
            set &= connection.ClearProxy();
        } else {
            set &= connection.EnableProxy();
        }
    }

    if (false == set) { LogError()()("Unable to reset connection.").Flush(); }

    return set;
}

auto ZeroMQPrivate::SocksProxy(UnallocatedCString& proxy) const noexcept -> bool
{
    const auto lock = Lock{lock_};
    proxy = socks_proxy_;

    return (!socks_proxy_.empty());
}

auto ZeroMQPrivate::SocksProxy() const noexcept -> UnallocatedCString
{
    UnallocatedCString output{};
    SocksProxy(output);

    return output;
}

auto ZeroMQPrivate::Status(const identifier::Notary& server) const noexcept
    -> opentxs::network::ConnectionState
{
    auto lock = Lock{lock_};
    const auto it = server_connections_.find(server);
    const bool haveConnection = it != server_connections_.end();
    lock.unlock();

    if (haveConnection) {
        if (it->second.Status()) {

            return opentxs::network::ConnectionState::ACTIVE;
        } else {

            return opentxs::network::ConnectionState::STALLED;
        }
    }

    return opentxs::network::ConnectionState::NOT_ESTABLISHED;
}

auto ZeroMQPrivate::verify_lock(const Lock& lock) const noexcept -> bool
{
    if (lock.mutex() != &lock_) {
        LogError()()("Incorrect mutex.").Flush();

        return false;
    }

    if (false == lock.owns_lock()) {
        LogError()()("Lock not owned.").Flush();

        return false;
    }

    return true;
}

ZeroMQPrivate::~ZeroMQPrivate() { server_connections_.clear(); }
}  // namespace opentxs::api::session

#undef CLIENT_SEND_TIMEOUT_SECONDS
#undef CLIENT_RECV_TIMEOUT_SECONDS
#undef CLIENT_SOCKET_LINGER_SECONDS
#undef CLIENT_SEND_TIMEOUT
#undef CLIENT_RECV_TIMEOUT
#undef KEEP_ALIVE_SECONDS
