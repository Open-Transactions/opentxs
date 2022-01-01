// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <memory>
#include <memory_resource>
#include <set>
#include <string>

#include "internal/blockchain/p2p/P2P.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/p2p/Address.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"

namespace opentxs
{
namespace api
{
class Session;
}  // namespace api
}  // namespace opentxs

namespace opentxs::blockchain::p2p::implementation
{
class Address final : public internal::Address
{
public:
    static const VersionNumber DefaultVersion;

    static auto instantiate_services(const SerializedType& serialized) noexcept
        -> std::pmr::set<Service>;

    auto Bytes() const noexcept -> OTData final { return bytes_; }
    auto Chain() const noexcept -> blockchain::Type final { return chain_; }
    auto Display() const noexcept -> std::string final;
    auto ID() const noexcept -> const Identifier& final { return id_; }
    auto Incoming() const noexcept -> bool final { return incoming_; }
    auto LastConnected() const noexcept -> Time final
    {
        return last_connected_;
    }
    auto Port() const noexcept -> std::uint16_t final { return port_; }
    auto PreviousLastConnected() const noexcept -> Time final
    {
        return previous_last_connected_;
    }
    auto PreviousServices() const noexcept -> std::pmr::set<Service> final
    {
        return previous_services_;
    }
    auto Serialize(SerializedType& out) const noexcept -> bool final;
    auto Services() const noexcept -> std::pmr::set<Service> final
    {
        return services_;
    }
    auto Style() const noexcept -> Protocol final { return protocol_; }
    auto Type() const noexcept -> Network final { return network_; }

    void AddService(const Service service) noexcept final
    {
        services_.emplace(service);
    }
    void RemoveService(const Service service) noexcept final
    {
        services_.erase(service);
    }
    void SetLastConnected(const Time& time) noexcept final
    {
        last_connected_ = time;
    }
    void SetServices(const std::pmr::set<Service>& services) noexcept final
    {
        services_ = services;
    }

    Address(
        const api::Session& api,
        const VersionNumber version,
        const Protocol protocol,
        const Network network,
        const ReadView bytes,
        const std::uint16_t port,
        const blockchain::Type chain,
        const Time lastConnected,
        const std::pmr::set<Service>& services,
        const bool incoming) noexcept(false);
    Address(const Address& rhs) noexcept;

    ~Address() final = default;

private:
    const api::Session& api_;
    const VersionNumber version_;
    const OTIdentifier id_;
    const Protocol protocol_;
    const Network network_;
    const OTData bytes_;
    const std::uint16_t port_;
    const blockchain::Type chain_;
    const Time previous_last_connected_;
    const std::pmr::set<Service> previous_services_;
    const bool incoming_;
    Time last_connected_;
    std::pmr::set<Service> services_;

    static auto calculate_id(
        const api::Session& api,
        const VersionNumber version,
        const Protocol protocol,
        const Network network,
        const ReadView bytes,
        const std::uint16_t port,
        const blockchain::Type chain) noexcept -> OTIdentifier;
    static auto serialize(
        const VersionNumber version,
        const Protocol protocol,
        const Network network,
        const ReadView bytes,
        const std::uint16_t port,
        const blockchain::Type chain,
        const Time lastConnected,
        const std::pmr::set<Service>& services) noexcept -> SerializedType;

    auto clone() const noexcept -> Address* final { return new Address(*this); }
    auto clone_internal() const noexcept
        -> std::unique_ptr<internal::Address> final
    {
        return std::make_unique<Address>(*this);
    }

    Address() = delete;
    Address(Address&&) = delete;
    auto operator=(const Address&) -> Address& = delete;
    auto operator=(Address&&) -> Address& = delete;
};
}  // namespace opentxs::blockchain::p2p::implementation
