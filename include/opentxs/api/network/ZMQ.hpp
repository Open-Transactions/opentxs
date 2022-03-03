// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <chrono>
#include <memory>

#include "opentxs/Types.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq

class ServerConnection;
}  // namespace network

class Flag;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::network
{
class OPENTXS_EXPORT ZMQ
{
public:
    virtual auto Context() const
        -> const opentxs::network::zeromq::Context& = 0;
    virtual auto DefaultAddressType() const -> AddressType = 0;
    virtual auto KeepAlive() const -> std::chrono::seconds = 0;
    virtual void KeepAlive(const std::chrono::seconds duration) const = 0;
    virtual auto Linger() const -> std::chrono::seconds = 0;
    virtual auto ReceiveTimeout() const -> std::chrono::seconds = 0;
    virtual auto Running() const -> const Flag& = 0;
    virtual void RefreshConfig() const = 0;
    virtual auto SendTimeout() const -> std::chrono::seconds = 0;
    virtual auto Server(const UnallocatedCString& id) const
        -> opentxs::network::ServerConnection& = 0;
    virtual auto SetSocksProxy(const UnallocatedCString& proxy) const
        -> bool = 0;
    virtual auto SocksProxy() const -> UnallocatedCString = 0;
    virtual auto SocksProxy(UnallocatedCString& proxy) const -> bool = 0;
    virtual auto Status(const UnallocatedCString& server) const
        -> ConnectionState = 0;

    OPENTXS_NO_EXPORT virtual ~ZMQ() = default;

protected:
    ZMQ() = default;

private:
    ZMQ(const ZMQ&) = delete;
    ZMQ(ZMQ&&) = delete;
    auto operator=(const ZMQ&) -> ZMQ& = delete;
    auto operator=(const ZMQ&&) -> ZMQ& = delete;
};
}  // namespace opentxs::api::network
