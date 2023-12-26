// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <chrono>
#include <mutex>

#include "internal/network/ServerConnection.hpp"
#include "internal/network/zeromq/socket/Publish.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/session/ZeroMQ.internal.hpp"
#include "opentxs/identifier/Notary.hpp"
#include "opentxs/network/Types.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class ZeroMQPrivate;  // IWYU pragma: keep
}  // namespace session

class Session;
}  // namespace api

class Flag;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class opentxs::api::session::ZeroMQPrivate final
    : virtual public internal::ZeroMQ
{
public:
    auto Context() const noexcept
        -> const opentxs::network::zeromq::Context& final;
    auto DefaultAddressType() const noexcept -> AddressType final;
    auto KeepAlive() const noexcept -> std::chrono::seconds final;
    auto KeepAlive(const std::chrono::seconds duration) const noexcept
        -> void final;
    auto Linger() const noexcept -> std::chrono::seconds final;
    auto ReceiveTimeout() const noexcept -> std::chrono::seconds final;
    auto RefreshConfig() const noexcept -> void final;
    auto Running() const noexcept -> const Flag& final;
    auto SendTimeout() const noexcept -> std::chrono::seconds final;
    auto Server(const identifier::Notary& id) const noexcept(false)
        -> opentxs::network::ServerConnection& final;
    auto SetSocksProxy(const UnallocatedCString& proxy) const noexcept
        -> bool final;
    auto SocksProxy() const noexcept -> UnallocatedCString final;
    auto SocksProxy(UnallocatedCString& proxy) const noexcept -> bool final;
    auto Status(const identifier::Notary& id) const noexcept
        -> opentxs::network::ConnectionState final;

    ZeroMQPrivate(const api::Session& api, const Flag& running) noexcept;
    ZeroMQPrivate() = delete;
    ZeroMQPrivate(const ZeroMQPrivate&) = delete;
    ZeroMQPrivate(ZeroMQPrivate&&) = delete;
    auto operator=(const ZeroMQPrivate&) -> ZeroMQPrivate& = delete;
    auto operator=(const ZeroMQPrivate&&) -> ZeroMQPrivate& = delete;

    ~ZeroMQPrivate() final;

private:
    const api::Session& api_;
    const Flag& running_;
    mutable std::atomic<std::chrono::seconds> linger_;
    mutable std::atomic<std::chrono::seconds> receive_timeout_;
    mutable std::atomic<std::chrono::seconds> send_timeout_;
    mutable std::atomic<std::chrono::seconds> keep_alive_;
    mutable std::mutex lock_;
    mutable UnallocatedCString socks_proxy_;
    mutable UnallocatedMap<
        identifier::Notary,
        opentxs::network::ServerConnection>
        server_connections_;
    OTZMQPublishSocket status_publisher_;

    auto verify_lock(const Lock& lock) const noexcept -> bool;

    auto init(const Lock& lock) const noexcept -> void;
};
