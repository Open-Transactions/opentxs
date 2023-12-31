// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/Factory.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/BlockchainPeerAddress.pb.h>
#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>

#include "network/blockchain/address/AddressPrivate.hpp"
#include "opentxs/Time.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/network/blockchain/Address.hpp"
#include "opentxs/network/blockchain/Transport.hpp"  // IWYU pragma: keep
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto BlockchainAddress(
    const api::Crypto& crypto,
    const api::Factory& factory,
    const network::blockchain::Protocol protocol,
    const network::blockchain::Transport type,
    const network::blockchain::Transport subtype,
    const ReadView bytes,
    const std::uint16_t port,
    const opentxs::blockchain::Type chain,
    const Time lastConnected,
    const Set<network::blockchain::bitcoin::Service>& services,
    const bool incoming,
    const ReadView cookie) noexcept -> network::blockchain::Address
{
    using ReturnType = network::blockchain::AddressPrivate;
    using enum network::blockchain::Transport;

    try {
        return std::make_unique<ReturnType>(
                   crypto,
                   factory,
                   ReturnType::DefaultVersion,
                   protocol,
                   type,
                   subtype,
                   ReadView{},
                   bytes,
                   port,
                   chain,
                   lastConnected,
                   services,
                   incoming,
                   cookie)
            .release();
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return {};
    }
}

auto BlockchainAddress(
    const api::Crypto& crypto,
    const api::Factory& factory,
    const network::blockchain::Protocol protocol,
    const network::blockchain::Transport type,
    const network::blockchain::Transport subtype,
    const ReadView key,
    const ReadView bytes,
    const std::uint16_t port,
    const blockchain::Type chain,
    const Time lastConnected,
    const Set<network::blockchain::bitcoin::Service>& services,
    const bool incoming,
    const ReadView cookie) noexcept -> network::blockchain::Address
{
    using ReturnType = network::blockchain::AddressPrivate;

    try {
        return std::make_unique<ReturnType>(
                   crypto,
                   factory,
                   ReturnType::DefaultVersion,
                   protocol,
                   type,
                   subtype,
                   key,
                   bytes,
                   port,
                   chain,
                   lastConnected,
                   services,
                   incoming,
                   cookie)
            .release();
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return {};
    }
}

auto BlockchainAddress(
    const api::Crypto& crypto,
    const api::Factory& factory,
    const protobuf::BlockchainPeerAddress& serialized) noexcept
    -> network::blockchain::Address
{
    using ReturnType = network::blockchain::AddressPrivate;

    try {
        return std::make_unique<ReturnType>(
                   crypto,
                   factory,
                   serialized.version(),
                   static_cast<network::blockchain::Protocol>(
                       serialized.protocol()),
                   static_cast<network::blockchain::Transport>(
                       serialized.network()),
                   static_cast<network::blockchain::Transport>(
                       serialized.subtype()),
                   serialized.key(),
                   serialized.address(),
                   static_cast<std::uint16_t>(serialized.port()),
                   static_cast<blockchain::Type>(serialized.chain()),
                   seconds_since_epoch_unsigned(serialized.time()).value(),
                   ReturnType::instantiate_services(serialized),
                   false,
                   ReadView{})
            .release();
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::factory
