// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                       // IWYU pragma: associated
#include "blockchain/p2p/AddressPrivate.hpp"  // IWYU pragma: associated

#include <BlockchainPeerAddress.pb.h>
#include <boost/asio.hpp>
#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>

#include "internal/api/session/FactoryAPI.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Time.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/p2p/Address.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::blockchain::p2p::implementation
{
class Address final : public AddressPrivate
{
public:
    static const VersionNumber DefaultVersion;

    static auto instantiate_services(
        const proto::BlockchainPeerAddress& serialized) noexcept
        -> UnallocatedSet<Service>
    {
        auto output = UnallocatedSet<Service>{};

        for (const auto& service : serialized.service()) {
            output.emplace(static_cast<Service>(service));
        }

        return output;
    }

    auto Bytes() const noexcept -> ByteArray final { return bytes_; }
    auto Chain() const noexcept -> blockchain::Type final { return chain_; }
    auto Display() const noexcept -> UnallocatedCString final
    {
        UnallocatedCString output{};

        switch (network_) {
            case Network::ipv4: {
                ip::address_v4::bytes_type bytes{};
                std::memcpy(bytes.data(), bytes_.data(), bytes.size());
                auto address = ip::make_address_v4(bytes);
                output = address.to_string();
            } break;
            case Network::ipv6:
            case Network::cjdns: {
                ip::address_v6::bytes_type bytes{};
                std::memcpy(bytes.data(), bytes_.data(), bytes.size());
                auto address = ip::make_address_v6(bytes);
                output = UnallocatedCString("[") + address.to_string() + "]";
            } break;
            case Network::onion2:
            case Network::onion3: {
                output = UnallocatedCString(
                             static_cast<const char*>(bytes_.data()),
                             bytes_.size()) +
                         ".onion";
            } break;
            case Network::eep: {
                // TODO handle errors
                [[maybe_unused]] const auto rc =
                    api_.Crypto().Encode().Base64Encode(
                        bytes_.Bytes(), writer(output));
                output += ".i2p";
            } break;
            case Network::zmq: {
                output = bytes_.Bytes();
            } break;
            default: {
                OT_FAIL;
            }
        }

        return output + ":" + std::to_string(port_);
    }
    auto ID() const noexcept -> const identifier::Generic& final { return id_; }
    auto Incoming() const noexcept -> bool final { return incoming_; }
    auto IsValid() const noexcept -> bool final { return true; }
    auto LastConnected() const noexcept -> Time final
    {
        return last_connected_;
    }
    auto Port() const noexcept -> std::uint16_t final { return port_; }
    auto PreviousLastConnected() const noexcept -> Time final
    {
        return previous_last_connected_;
    }
    auto PreviousServices() const noexcept -> UnallocatedSet<Service> final
    {
        return previous_services_;
    }
    auto Serialize(proto::BlockchainPeerAddress& out) const noexcept
        -> bool final
    {
        out = serialize(
            version_,
            protocol_,
            network_,
            bytes_.Bytes(),
            port_,
            chain_,
            last_connected_,
            services_);
        out.set_id(id_.asBase58(api_.Crypto()));

        return true;
    }
    auto Services() const noexcept -> UnallocatedSet<Service> final
    {
        return services_;
    }
    auto Style() const noexcept -> Protocol final { return protocol_; }
    auto Type() const noexcept -> Network final { return network_; }

    auto AddService(const Service service) noexcept -> void final
    {
        services_.emplace(service);
    }
    auto RemoveService(const Service service) noexcept -> void final
    {
        services_.erase(service);
    }
    auto SetIncoming(bool value) noexcept -> void final { incoming_ = value; }
    auto SetLastConnected(const Time& time) noexcept -> void final
    {
        last_connected_ = time;
    }
    auto SetServices(const UnallocatedSet<Service>& services) noexcept
        -> void final
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
        const UnallocatedSet<Service>& services,
        const bool incoming) noexcept(false)
        : api_(api)
        , version_(version)
        , id_(calculate_id(api, version, protocol, network, bytes, port, chain))
        , protocol_(protocol)
        , network_(network)
        , bytes_(api.Factory().DataFromBytes(bytes))
        , port_(port)
        , chain_(chain)
        , previous_last_connected_(lastConnected)
        , previous_services_(services)
        , incoming_(incoming)
        , last_connected_(lastConnected)
        , services_(services)
    {
        const auto size = bytes_.size();

        switch (network_) {
            case Network::ipv4: {
                if (sizeof(ip::address_v4::bytes_type) != size) {
                    throw std::runtime_error("Incorrect ipv4 bytes");
                }
            } break;
            case Network::ipv6:
            case Network::cjdns: {
                if (sizeof(ip::address_v6::bytes_type) != size) {
                    throw std::runtime_error("Incorrect ipv6 bytes");
                }
            } break;
            case Network::onion2: {
                if (10 != size) {
                    throw std::runtime_error("Incorrect onion bytes");
                }
            } break;
            case Network::onion3: {
                if (56 != size) {
                    throw std::runtime_error("Incorrect onion bytes");
                }
            } break;
            case Network::eep: {
                if (32 != size) {  // TODO replace ths with correct value
                    throw std::runtime_error("Incorrect eep bytes");
                }
            } break;
            case Network::zmq: {
            } break;
            default: {
                OT_FAIL;
            }
        }
    }

    Address() = delete;
    Address(const Address& rhs) noexcept
        : api_(rhs.api_)
        , version_(rhs.version_)
        , id_(rhs.id_)
        , protocol_(rhs.protocol_)
        , network_(rhs.network_)
        , bytes_(rhs.bytes_)
        , port_(rhs.port_)
        , chain_(rhs.chain_)
        , previous_last_connected_(rhs.previous_last_connected_)
        , previous_services_(rhs.previous_services_)
        , incoming_(rhs.incoming_)
        , last_connected_(rhs.last_connected_)
        , services_(rhs.services_)
    {
    }
    Address(Address&&) = delete;
    auto operator=(const Address&) -> Address& = delete;
    auto operator=(Address&&) -> Address& = delete;

    ~Address() final = default;

private:
    const api::Session& api_;
    const VersionNumber version_;
    const identifier::Generic id_;
    const Protocol protocol_;
    const Network network_;
    const ByteArray bytes_;
    const std::uint16_t port_;
    const blockchain::Type chain_;
    const Time previous_last_connected_;
    const UnallocatedSet<Service> previous_services_;
    bool incoming_;
    Time last_connected_;
    UnallocatedSet<Service> services_;

    static auto calculate_id(
        const api::Session& api,
        const VersionNumber version,
        const Protocol protocol,
        const Network network,
        const ReadView bytes,
        const std::uint16_t port,
        const blockchain::Type chain) noexcept -> identifier::Generic
    {
        const auto serialized = serialize(
            version,
            protocol,
            network,
            bytes,
            port,
            chain,
            convert_stime(0),
            {});

        return api.Factory().InternalSession().IdentifierFromPreimage(
            serialized);
    }
    static auto serialize(
        const VersionNumber version,
        const Protocol protocol,
        const Network network,
        const ReadView bytes,
        const std::uint16_t port,
        const blockchain::Type chain,
        const Time lastConnected,
        const UnallocatedSet<Service>& services) noexcept
        -> proto::BlockchainPeerAddress
    {
        auto output = proto::BlockchainPeerAddress{};
        output.set_version(version);
        output.set_protocol(static_cast<std::uint8_t>(protocol));
        output.set_network(static_cast<std::uint8_t>(network));
        output.set_chain(static_cast<std::uint32_t>(chain));
        output.set_address(bytes.data(), bytes.size());
        output.set_port(port);
        output.set_time(Clock::to_time_t(lastConnected));

        for (const auto& service : services) {
            output.add_service(static_cast<std::uint8_t>(service));
        }

        return output;
    }
};

const VersionNumber Address::DefaultVersion{1};
}  // namespace opentxs::blockchain::p2p::implementation

namespace opentxs::factory
{
auto BlockchainAddress(
    const api::Session& api,
    const blockchain::p2p::Protocol protocol,
    const blockchain::p2p::Network network,
    const Data& bytes,
    const std::uint16_t port,
    const blockchain::Type chain,
    const Time lastConnected,
    const UnallocatedSet<blockchain::p2p::Service>& services,
    const bool incoming) noexcept -> blockchain::p2p::Address
{
    using ReturnType = blockchain::p2p::implementation::Address;

    try {
        return std::make_unique<ReturnType>(
                   api,
                   ReturnType::DefaultVersion,
                   protocol,
                   network,
                   bytes.Bytes(),
                   port,
                   chain,
                   lastConnected,
                   services,
                   incoming)
            .release();
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto BlockchainAddress(
    const api::Session& api,
    const proto::BlockchainPeerAddress& serialized) noexcept
    -> blockchain::p2p::Address
{
    using ReturnType = blockchain::p2p::implementation::Address;

    try {
        return std::make_unique<ReturnType>(
                   api,
                   serialized.version(),
                   static_cast<blockchain::p2p::Protocol>(
                       serialized.protocol()),
                   static_cast<blockchain::p2p::Network>(serialized.network()),
                   serialized.address(),
                   static_cast<std::uint16_t>(serialized.port()),
                   static_cast<blockchain::Type>(serialized.chain()),
                   convert_stime(serialized.time()),
                   ReturnType::instantiate_services(serialized),
                   false)
            .release();
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::factory
