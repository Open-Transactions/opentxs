// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>

#include "opentxs/Export.hpp"
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
class ZeroMQ;
}  // namespace internal

class ZeroMQ;  // IWYU pragma: keep
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

class OPENTXS_EXPORT opentxs::api::session::ZeroMQ
{
public:
    auto Context() const noexcept -> const opentxs::network::zeromq::Context&;
    auto DefaultAddressType() const noexcept -> AddressType;
    OPENTXS_NO_EXPORT auto Internal() const noexcept -> const internal::ZeroMQ&;
    auto KeepAlive() const noexcept -> std::chrono::seconds;
    auto KeepAlive(const std::chrono::seconds duration) const noexcept -> void;
    auto Linger() const noexcept -> std::chrono::seconds;
    auto ReceiveTimeout() const noexcept -> std::chrono::seconds;
    auto Running() const noexcept -> const Flag&;
    auto RefreshConfig() const noexcept -> void;
    auto SendTimeout() const noexcept -> std::chrono::seconds;
    auto Server(const identifier::Notary& id) const noexcept(false)
        -> opentxs::network::ServerConnection&;
    auto SetSocksProxy(const UnallocatedCString& proxy) const noexcept -> bool;
    auto SocksProxy() const noexcept -> UnallocatedCString;
    auto SocksProxy(UnallocatedCString& proxy) const noexcept -> bool;
    auto Status(const identifier::Notary& id) const noexcept
        -> opentxs::network::ConnectionState;

    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::ZeroMQ&;

    OPENTXS_NO_EXPORT ZeroMQ(internal::ZeroMQ* imp) noexcept;
    ZeroMQ() = delete;
    ZeroMQ(const ZeroMQ&) = delete;
    ZeroMQ(ZeroMQ&&) = delete;
    auto operator=(const ZeroMQ&) -> ZeroMQ& = delete;
    auto operator=(const ZeroMQ&&) -> ZeroMQ& = delete;

    OPENTXS_NO_EXPORT virtual ~ZeroMQ();

protected:
    friend internal::ZeroMQ;

    internal::ZeroMQ* imp_;
};
