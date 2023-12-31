// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <memory>

#include "internal/network/blockchain/Address.hpp"
#include "opentxs/Time.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
class Factory;
}  // namespace api

namespace protobuf
{
class BlockchainPeerAddress;
}  // namespace protobuf
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::blockchain
{
class AddressPrivate final : public internal::Address
{
public:
    static const VersionNumber DefaultVersion;

    static auto instantiate_services(
        const protobuf::BlockchainPeerAddress& serialized) noexcept
        -> Set<bitcoin::Service>;

    auto Bytes() const noexcept -> ByteArray final;
    auto Chain() const noexcept -> opentxs::blockchain::Type final;
    auto clone() const noexcept -> std::unique_ptr<Address> final;
    auto Cookie() const noexcept -> ReadView final;
    auto Display() const noexcept -> UnallocatedCString final;
    auto ID() const noexcept -> const identifier::Generic& final;
    auto Incoming() const noexcept -> bool final;
    auto IsValid() const noexcept -> bool final;
    auto Key() const noexcept -> ReadView final;
    auto LastConnected() const noexcept -> Time final;
    auto Port() const noexcept -> std::uint16_t final;
    auto PreviousLastConnected() const noexcept -> Time final;
    auto PreviousServices() const noexcept -> Set<bitcoin::Service> final;
    auto Serialize(protobuf::BlockchainPeerAddress& out) const noexcept
        -> bool final;
    auto Services() const noexcept -> Set<bitcoin::Service> final;
    auto Subtype() const noexcept -> Transport final;
    auto Style() const noexcept -> Protocol final;
    auto Type() const noexcept -> Transport final;

    auto AddService(const bitcoin::Service service) noexcept -> void final;
    auto RemoveService(const bitcoin::Service service) noexcept -> void final;
    auto SetIncoming(bool value) noexcept -> void final;
    auto SetLastConnected(const Time& time) noexcept -> void final;
    auto SetServices(const Set<bitcoin::Service>& services) noexcept
        -> void final;

    AddressPrivate(
        const api::Crypto& crypto,
        const api::Factory& factory,
        const VersionNumber version,
        const Protocol protocol,
        const Transport network,
        const Transport subtype,
        const ReadView key,
        const ReadView bytes,
        const std::uint16_t port,
        const opentxs::blockchain::Type chain,
        const Time lastConnected,
        const Set<bitcoin::Service>& services,
        const bool incoming,
        const ReadView cookie) noexcept(false);

    AddressPrivate() = delete;
    AddressPrivate(const AddressPrivate& rhs) noexcept;
    AddressPrivate(AddressPrivate&&) = delete;
    auto operator=(const AddressPrivate&) -> AddressPrivate& = delete;
    auto operator=(AddressPrivate&&) -> AddressPrivate& = delete;

    ~AddressPrivate() final;

private:
    const api::Crypto& crypto_;
    const VersionNumber version_;
    const identifier::Generic id_;
    const Protocol protocol_;
    const Transport type_;
    const Transport subtype_;
    const ByteArray key_;
    const ByteArray bytes_;
    const std::uint16_t port_;
    const opentxs::blockchain::Type chain_;
    const Time previous_last_connected_;
    const Set<bitcoin::Service> previous_services_;
    const ByteArray cookie_;
    bool incoming_;
    Time last_connected_;
    Set<bitcoin::Service> services_;

    static auto calculate_id(
        const api::Factory& api,
        const VersionNumber version,
        const Protocol protocol,
        const Transport network,
        const Transport subtype,
        const ReadView key,
        const ReadView bytes,
        const std::uint16_t port,
        const opentxs::blockchain::Type chain) noexcept -> identifier::Generic;
    static auto serialize(
        const VersionNumber version,
        const Protocol protocol,
        const Transport network,
        const Transport subtype,
        const ReadView key,
        const ReadView bytes,
        const std::uint16_t port,
        const opentxs::blockchain::Type chain,
        const Time lastConnected,
        const Set<bitcoin::Service>& services) noexcept
        -> protobuf::BlockchainPeerAddress;
};
}  // namespace opentxs::network::blockchain
