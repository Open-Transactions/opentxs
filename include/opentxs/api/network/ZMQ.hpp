// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_NETWORK_ZMQ_HPP
#define OPENTXS_API_NETWORK_ZMQ_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <chrono>
#include <memory>
#include <string>

#include "opentxs/Types.hpp"
#include "opentxs/core/Types.hpp"

namespace opentxs
{
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

namespace opentxs
{
namespace api
{
namespace network
{
class OPENTXS_EXPORT ZMQ
{
public:
    virtual const opentxs::network::zeromq::Context& Context() const = 0;
    virtual core::AddressType DefaultAddressType() const = 0;
    virtual std::chrono::seconds KeepAlive() const = 0;
    virtual void KeepAlive(const std::chrono::seconds duration) const = 0;
    virtual std::chrono::seconds Linger() const = 0;
    virtual std::chrono::seconds ReceiveTimeout() const = 0;
    virtual const Flag& Running() const = 0;
    virtual void RefreshConfig() const = 0;
    virtual std::chrono::seconds SendTimeout() const = 0;
    virtual opentxs::network::ServerConnection& Server(
        const std::string& id) const = 0;
    virtual bool SetSocksProxy(const std::string& proxy) const = 0;
    virtual std::string SocksProxy() const = 0;
    virtual bool SocksProxy(std::string& proxy) const = 0;
    virtual ConnectionState Status(const std::string& server) const = 0;

    OPENTXS_NO_EXPORT virtual ~ZMQ() = default;

protected:
    ZMQ() = default;

private:
    ZMQ(const ZMQ&) = delete;
    ZMQ(ZMQ&&) = delete;
    ZMQ& operator=(const ZMQ&) = delete;
    ZMQ& operator=(const ZMQ&&) = delete;
};
}  // namespace network
}  // namespace api
}  // namespace opentxs
#endif
