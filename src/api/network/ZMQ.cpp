// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "api/network/ZMQ.hpp"  // IWYU pragma: associated

#include <atomic>
#include <cstdint>
#include <utility>

#include "2_Factory.hpp"
#include "internal/api/Settings.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/core/String.hpp"
#include "internal/network/ServerConnection.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/socket/Publish.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/AddressType.hpp"  // IWYU pragma: keep
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Notary.hpp"  // IWYU pragma: keep
#include "opentxs/network/Types.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

#define CLIENT_SEND_TIMEOUT_SECONDS 5
#if OT_VALGRIND
#define CLIENT_RECV_TIMEOUT_SECONDS 50
#else
#define CLIENT_RECV_TIMEOUT_SECONDS 5
#endif
#define CLIENT_SOCKET_LINGER_SECONDS 0
#define CLIENT_SEND_TIMEOUT CLIENT_SEND_TIMEOUT_SECONDS
#define CLIENT_RECV_TIMEOUT CLIENT_RECV_TIMEOUT_SECONDS
#define KEEP_ALIVE_SECONDS 30

namespace opentxs
{
auto Factory::ZMQ(const api::Session& api, const Flag& running)
    -> api::network::ZMQ*
{
    return new api::network::imp::ZMQ(api, running);
}
}  // namespace opentxs

namespace opentxs::api::network::imp
{
ZMQ::ZMQ(const api::Session& api, const Flag& running)
    : api_(api)
    , running_(running)
    , linger_(std::chrono::seconds(CLIENT_SOCKET_LINGER_SECONDS))
    , receive_timeout_(std::chrono::seconds(CLIENT_RECV_TIMEOUT))
    , send_timeout_(std::chrono::seconds(CLIENT_SEND_TIMEOUT))
    , keep_alive_(0s)
    , lock_()
    , socks_proxy_()
    , server_connections_()
    , status_publisher_(api_.Network().ZeroMQ().Internal().PublishSocket())
{
    // WARNING: do not access api_.Wallet() during construction
    status_publisher_->Start(api_.Endpoints().ConnectionStatus().data());

    Lock lock(lock_);

    init(lock);
}

auto ZMQ::Context() const -> const opentxs::network::zeromq::Context&
{
    return api_.Network().ZeroMQ();
}

auto ZMQ::DefaultAddressType() const -> AddressType
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
        LogAbort()(OT_PRETTY_CLASS())("failed to save config file").Abort();
    }

    return static_cast<AddressType>(configuredType);
}

void ZMQ::init(const Lock& lock) const
{
    OT_ASSERT(verify_lock(lock));

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
        LogAbort()(OT_PRETTY_CLASS())("failed to save config file").Abort();
    }
}

auto ZMQ::KeepAlive() const -> std::chrono::seconds
{
    return keep_alive_.load();
}

void ZMQ::KeepAlive(const std::chrono::seconds duration) const
{
    keep_alive_.store(duration);
}

auto ZMQ::Linger() const -> std::chrono::seconds { return linger_.load(); }

auto ZMQ::ReceiveTimeout() const -> std::chrono::seconds
{
    return receive_timeout_.load();
}

void ZMQ::RefreshConfig() const
{
    Lock lock(lock_);

    return init(lock);
}

auto ZMQ::Running() const -> const Flag& { return running_; }

auto ZMQ::SendTimeout() const -> std::chrono::seconds
{
    return send_timeout_.load();
}

auto ZMQ::Server(const UnallocatedCString& id) const noexcept(false)
    -> opentxs::network::ServerConnection&
{
    Lock lock(lock_);
    auto existing = server_connections_.find(id);

    if (server_connections_.end() != existing) { return existing->second; }

    auto contract =
        api_.Wallet().Internal().Server(api_.Factory().NotaryIDFromBase58(id));
    auto [it, created] = server_connections_.emplace(
        id,
        opentxs::network::ServerConnection::Factory(
            api_, *this, status_publisher_, contract));
    auto& connection = it->second;

    OT_ASSERT(created);

    if (false == socks_proxy_.empty()) { connection.EnableProxy(); }

    return connection;
}

auto ZMQ::SetSocksProxy(const UnallocatedCString& proxy) const -> bool
{
    bool notUsed{false};
    bool set = api_.Config().Internal().Set_str(
        String::Factory("Connection"),
        String::Factory("socks_proxy"),
        String::Factory(proxy),
        notUsed);

    if (false == set) {
        LogError()(OT_PRETTY_CLASS())("Unable to set socks proxy.").Flush();

        return false;
    }

    if (false == api_.Config().Internal().Save()) {
        LogError()(OT_PRETTY_CLASS())("Unable to set save config.").Flush();

        return false;
    }

    Lock lock(lock_);
    socks_proxy_ = proxy;

    for (auto& it : server_connections_) {
        opentxs::network::ServerConnection& connection = it.second;

        if (proxy.empty()) {
            set &= connection.ClearProxy();
        } else {
            set &= connection.EnableProxy();
        }
    }

    if (false == set) {
        LogError()(OT_PRETTY_CLASS())("Unable to reset connection.").Flush();
    }

    return set;
}

auto ZMQ::SocksProxy(UnallocatedCString& proxy) const -> bool
{
    Lock lock(lock_);
    proxy = socks_proxy_;

    return (!socks_proxy_.empty());
}

auto ZMQ::SocksProxy() const -> UnallocatedCString
{
    UnallocatedCString output{};
    SocksProxy(output);

    return output;
}

auto ZMQ::Status(const UnallocatedCString& server) const
    -> opentxs::network::ConnectionState
{
    Lock lock(lock_);
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

auto ZMQ::verify_lock(const Lock& lock) const -> bool
{
    if (lock.mutex() != &lock_) {
        LogError()(OT_PRETTY_CLASS())("Incorrect mutex.").Flush();

        return false;
    }

    if (false == lock.owns_lock()) {
        LogError()(OT_PRETTY_CLASS())("Lock not owned.").Flush();

        return false;
    }

    return true;
}

ZMQ::~ZMQ() { server_connections_.clear(); }
}  // namespace opentxs::api::network::imp
