// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>

#include "opentxs/core/Types.hpp"
#include "opentxs/network/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
namespace internal
{
class ZeroMQ;  // IWYU pragma: keep
}  // namespace internal
}  // namespace session
}  // namespace api

namespace identifier
{
class Notary;
}  // namespace identifier

namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq

class ServerConnection;
}  // namespace network

class Flag;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class opentxs::api::session::internal::ZeroMQ
{
public:
    virtual auto Context() const noexcept
        -> const opentxs::network::zeromq::Context& = 0;
    virtual auto DefaultAddressType() const noexcept -> AddressType = 0;
    virtual auto KeepAlive() const noexcept -> std::chrono::seconds = 0;
    virtual auto KeepAlive(const std::chrono::seconds duration) const noexcept
        -> void = 0;
    virtual auto Linger() const noexcept -> std::chrono::seconds = 0;
    virtual auto ReceiveTimeout() const noexcept -> std::chrono::seconds = 0;
    virtual auto Running() const noexcept -> const Flag& = 0;
    virtual auto RefreshConfig() const noexcept -> void = 0;
    virtual auto SendTimeout() const noexcept -> std::chrono::seconds = 0;
    virtual auto Server(const identifier::Notary& id) const noexcept(false)
        -> opentxs::network::ServerConnection& = 0;
    virtual auto SetSocksProxy(const UnallocatedCString& proxy) const noexcept
        -> bool = 0;
    virtual auto SocksProxy() const noexcept -> UnallocatedCString = 0;
    virtual auto SocksProxy(UnallocatedCString& proxy) const noexcept
        -> bool = 0;
    virtual auto Status(const identifier::Notary& id) const noexcept
        -> opentxs::network::ConnectionState = 0;

    ZeroMQ() = default;
    ZeroMQ(const ZeroMQ&) = delete;
    ZeroMQ(ZeroMQ&&) = delete;
    auto operator=(const ZeroMQ&) -> ZeroMQ& = delete;
    auto operator=(const ZeroMQ&&) -> ZeroMQ& = delete;

    virtual ~ZeroMQ() = default;
};
