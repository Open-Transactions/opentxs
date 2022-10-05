// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>

#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/core/AddressType.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/network/ServerConnection.hpp"
#include "opentxs/network/Types.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

class Factory;
class Flag;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::network::imp
{
class ZMQ final : virtual public opentxs::api::network::ZMQ
{
public:
    auto Context() const -> const opentxs::network::zeromq::Context& final;
    auto DefaultAddressType() const -> AddressType final;
    auto KeepAlive() const -> std::chrono::seconds final;
    void KeepAlive(const std::chrono::seconds duration) const final;
    auto Linger() const -> std::chrono::seconds final;
    auto ReceiveTimeout() const -> std::chrono::seconds final;
    void RefreshConfig() const final;
    auto Running() const -> const Flag& final;
    auto SendTimeout() const -> std::chrono::seconds final;

    auto Server(const UnallocatedCString& id) const
        -> opentxs::network::ServerConnection& final;
    auto SetSocksProxy(const UnallocatedCString& proxy) const -> bool final;
    auto SocksProxy() const -> UnallocatedCString final;
    auto SocksProxy(UnallocatedCString& proxy) const -> bool final;
    auto Status(const UnallocatedCString& server) const
        -> opentxs::network::ConnectionState final;

    ZMQ() = delete;
    ZMQ(const ZMQ&) = delete;
    ZMQ(ZMQ&&) = delete;
    auto operator=(const ZMQ&) -> ZMQ& = delete;
    auto operator=(const ZMQ&&) -> ZMQ& = delete;

    ~ZMQ() final;

private:
    friend opentxs::Factory;

    const api::Session& api_;
    const Flag& running_;
    mutable std::atomic<std::chrono::seconds> linger_;
    mutable std::atomic<std::chrono::seconds> receive_timeout_;
    mutable std::atomic<std::chrono::seconds> send_timeout_;
    mutable std::atomic<std::chrono::seconds> keep_alive_;
    mutable std::mutex lock_;
    mutable UnallocatedCString socks_proxy_;
    mutable UnallocatedMap<
        UnallocatedCString,
        opentxs::network::ServerConnection>
        server_connections_;
    OTZMQPublishSocket status_publisher_;

    auto verify_lock(const Lock& lock) const -> bool;

    void init(const Lock& lock) const;

    ZMQ(const api::Session& api, const Flag& running);
};
}  // namespace opentxs::api::network::imp
