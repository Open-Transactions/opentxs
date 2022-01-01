// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <map>
#include <mutex>
#include <string>

#include "Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/core/AddressType.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/network/ServerConnection.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"

namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

class Factory;
class Flag;
}  // namespace opentxs

namespace opentxs::api::network::imp
{
class ZMQ final : virtual public opentxs::api::network::ZMQ
{
public:
    auto Context() const -> const opentxs::network::zeromq::Context& final;
    auto DefaultAddressType() const -> core::AddressType final;
    auto KeepAlive() const -> std::chrono::seconds final;
    void KeepAlive(const std::chrono::seconds duration) const final;
    auto Linger() const -> std::chrono::seconds final;
    auto ReceiveTimeout() const -> std::chrono::seconds final;
    void RefreshConfig() const final;
    auto Running() const -> const Flag& final;
    auto SendTimeout() const -> std::chrono::seconds final;

    auto Server(const std::string& id) const
        -> opentxs::network::ServerConnection& final;
    auto SetSocksProxy(const std::string& proxy) const -> bool final;
    auto SocksProxy() const -> std::string final;
    auto SocksProxy(std::string& proxy) const -> bool final;
    auto Status(const std::string& server) const -> ConnectionState final;

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
    mutable std::string socks_proxy_;
    mutable std::pmr::map<std::string, OTServerConnection> server_connections_;
    OTZMQPublishSocket status_publisher_;

    auto verify_lock(const Lock& lock) const -> bool;

    void init(const Lock& lock) const;

    ZMQ(const api::Session& api, const Flag& running);
    ZMQ() = delete;
    ZMQ(const ZMQ&) = delete;
    ZMQ(ZMQ&&) = delete;
    auto operator=(const ZMQ&) -> ZMQ& = delete;
    auto operator=(const ZMQ&&) -> ZMQ& = delete;
};
}  // namespace opentxs::api::network::imp
