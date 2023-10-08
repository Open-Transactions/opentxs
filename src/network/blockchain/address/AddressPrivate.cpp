// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/blockchain/address/AddressPrivate.hpp"  // IWYU pragma: associated

#include <BlockchainPeerAddress.pb.h>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>

#include "BoostAsio.hpp"
#include "internal/api/FactoryAPI.hpp"
#include "internal/network/asio/Types.hpp"
#include "internal/network/blockchain/bitcoin/message/Types.hpp"  // IWYU pragma: keep
#include "internal/util/P0330.hpp"
#include "internal/util/Time.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/network/blockchain/Transport.hpp"  // IWYU pragma: keep
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::network::blockchain
{
AddressPrivate::AddressPrivate(
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
    const ReadView cookie) noexcept(false)
    : crypto_(crypto)
    , version_(version)
    , id_(calculate_id(
          factory,
          version,
          protocol,
          network,
          subtype,
          key,
          bytes,
          port,
          chain))
    , protocol_(protocol)
    , type_(network)
    , subtype_(subtype)
    , key_(key)
    , bytes_(bytes)
    , port_(port)
    , chain_(chain)
    , previous_last_connected_(lastConnected)
    , previous_services_(services)
    , cookie_(cookie)
    , incoming_(incoming)
    , last_connected_(lastConnected)
    , services_(services)
{
    const auto size = bytes_.size();
    using enum Transport;

    switch (type_) {
        case ipv4: {
            if (sizeof(ip::address_v4::bytes_type) != size) {
                const auto error = UnallocatedCString{"expected "}
                                       .append(std::to_string(
                                           sizeof(ip::address_v4::bytes_type)))
                                       .append(" bytes for ipv4 but received ")
                                       .append(std::to_string(size))
                                       .append(" bytes");

                throw std::runtime_error(error);
            }
        } break;
        case ipv6:
        case cjdns: {
            if (sizeof(ip::address_v6::bytes_type) != size) {
                const auto error = UnallocatedCString{"expected "}
                                       .append(std::to_string(
                                           sizeof(ip::address_v6::bytes_type)))
                                       .append(" bytes for ipv4 but received ")
                                       .append(std::to_string(size))
                                       .append(" bytes");

                throw std::runtime_error(error);
            }
        } break;
        case onion2: {
            if (10_uz != size) {
                const auto error =
                    UnallocatedCString{"expected "}
                        .append(std::to_string(10_uz))
                        .append(" bytes for onion2 but received ")
                        .append(std::to_string(size))
                        .append(" bytes");

                throw std::runtime_error(error);
            }
        } break;
        case onion3: {
            if (32_uz != size) {
                const auto error =
                    UnallocatedCString{"expected "}
                        .append(std::to_string(32_uz))
                        .append(" bytes for onion3 but received ")
                        .append(std::to_string(size))
                        .append(" bytes");

                throw std::runtime_error(error);
            }
        } break;
        case eep: {
            if (32_uz != size) {
                const auto error = UnallocatedCString{"expected "}
                                       .append(std::to_string(32_uz))
                                       .append(" bytes for eep but received ")
                                       .append(std::to_string(size))
                                       .append(" bytes");

                throw std::runtime_error(error);
            }
        } break;
        case zmq: {
        } break;
        default: {
            LogAbort()()("unhandled transport type ")(print(type_)).Abort();
        }
    }
}

AddressPrivate::AddressPrivate(const AddressPrivate& rhs) noexcept
    : crypto_(rhs.crypto_)
    , version_(rhs.version_)
    , id_(rhs.id_)
    , protocol_(rhs.protocol_)
    , type_(rhs.type_)
    , subtype_(rhs.subtype_)
    , key_(rhs.key_)
    , bytes_(rhs.bytes_)
    , port_(rhs.port_)
    , chain_(rhs.chain_)
    , previous_last_connected_(rhs.previous_last_connected_)
    , previous_services_(rhs.previous_services_)
    , cookie_(rhs.cookie_)
    , incoming_(rhs.incoming_)
    , last_connected_(rhs.last_connected_)
    , services_(rhs.services_)
{
}

auto AddressPrivate::AddService(const bitcoin::Service service) noexcept -> void
{
    services_.emplace(service);
}

auto AddressPrivate::Bytes() const noexcept -> ByteArray { return bytes_; }

auto AddressPrivate::calculate_id(
    const api::Factory& api,
    const VersionNumber version,
    const Protocol protocol,
    const Transport network,
    const Transport subtype,
    const ReadView key,
    const ReadView bytes,
    const std::uint16_t port,
    const opentxs::blockchain::Type chain) noexcept -> identifier::Generic
{
    const auto serialized = serialize(
        version,
        protocol,
        network,
        subtype,
        key,
        bytes,
        port,
        chain,
        convert_stime(0),
        {});

    return api.Internal().IdentifierFromPreimage(serialized);
}

auto AddressPrivate::Chain() const noexcept -> opentxs::blockchain::Type
{
    return chain_;
}

auto AddressPrivate::clone() const noexcept -> std::unique_ptr<Address>
{
    return std::make_unique<AddressPrivate>(*this);
}

auto AddressPrivate::Cookie() const noexcept -> ReadView
{
    return cookie_.Bytes();
}

auto AddressPrivate::Display() const noexcept -> UnallocatedCString
{
    auto output = UnallocatedCString{};
    const auto print = [&]() {
        const auto address = asio::address_from_binary(bytes_.Bytes());

        if (address.has_value()) {

            output = address->to_string();
        } else {

            output = "invalid address";
        }
    };
    const auto printOnion = [&]() {
        output = UnallocatedCString(
                     static_cast<const char*>(bytes_.data()), bytes_.size()) +
                 ".onion";
    };
    const auto printEep = [&]() {
        // TODO handle errors
        [[maybe_unused]] const auto rc =
            crypto_.Encode().Base64Encode(bytes_.Bytes(), writer(output));
        output += ".i2p";
    };
    using enum Transport;
    auto includePort{true};

    switch (type_) {
        case ipv4:
        case ipv6:
        case cjdns: {
            std::invoke(print);
        } break;
        case onion2:
        case onion3: {
            std::invoke(printOnion);
        } break;
        case eep: {
            std::invoke(printEep);
        } break;
        case zmq: {
            switch (subtype_) {
                case ipv4:
                case ipv6:
                case cjdns: {
                    std::invoke(print);
                } break;
                case onion2:
                case onion3: {
                    std::invoke(printOnion);
                } break;
                case eep: {
                    std::invoke(printEep);
                } break;
                case zmq: {
                    output = bytes_.Bytes();
                    includePort = !incoming_;
                } break;
                default: {
                    output = "invalid address subtype";
                }
            }
        } break;
        default: {
            output = "invalid address type";
        }
    }

    if (includePort) { output.append(":").append(std::to_string(port_)); }

    return output;
}

auto AddressPrivate::ID() const noexcept -> const identifier::Generic&
{
    return id_;
}

auto AddressPrivate::Incoming() const noexcept -> bool { return incoming_; }

auto AddressPrivate::instantiate_services(
    const proto::BlockchainPeerAddress& serialized) noexcept
    -> Set<bitcoin::Service>
{
    auto output = Set<bitcoin::Service>{};

    for (const auto& service : serialized.service()) {
        output.emplace(static_cast<bitcoin::Service>(service));
    }

    return output;
}

auto AddressPrivate::IsValid() const noexcept -> bool { return true; }

auto AddressPrivate::Key() const noexcept -> ReadView { return key_.Bytes(); }

auto AddressPrivate::LastConnected() const noexcept -> Time
{
    return last_connected_;
}

auto AddressPrivate::Port() const noexcept -> std::uint16_t { return port_; }

auto AddressPrivate::PreviousLastConnected() const noexcept -> Time
{
    return previous_last_connected_;
}

auto AddressPrivate::PreviousServices() const noexcept -> Set<bitcoin::Service>
{
    return previous_services_;
}

auto AddressPrivate::RemoveService(const bitcoin::Service service) noexcept
    -> void
{
    services_.erase(service);
}

auto AddressPrivate::Serialize(proto::BlockchainPeerAddress& out) const noexcept
    -> bool
{
    out = serialize(
        version_,
        protocol_,
        type_,
        subtype_,
        key_.Bytes(),
        bytes_.Bytes(),
        port_,
        chain_,
        last_connected_,
        services_);
    out.set_id(id_.asBase58(crypto_));

    return true;
}

auto AddressPrivate::serialize(
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
    -> proto::BlockchainPeerAddress
{
    auto output = proto::BlockchainPeerAddress{};
    output.set_version(version);
    output.set_protocol(static_cast<std::uint8_t>(protocol));
    output.set_network(static_cast<std::uint8_t>(network));
    output.set_chain(static_cast<std::uint32_t>(chain));

    if (valid(bytes)) { output.set_address(bytes.data(), bytes.size()); }

    output.set_port(port);
    output.set_time(Clock::to_time_t(lastConnected));

    for (const auto& service : services) {
        output.add_service(static_cast<std::uint8_t>(service));
    }

    output.set_subtype(static_cast<std::uint8_t>(subtype));

    if (valid(key)) { output.set_key(key.data(), key.size()); }

    return output;
}

auto AddressPrivate::Services() const noexcept -> Set<bitcoin::Service>
{
    return services_;
}

auto AddressPrivate::SetIncoming(bool value) noexcept -> void
{
    incoming_ = value;
}

auto AddressPrivate::SetLastConnected(const Time& time) noexcept -> void
{
    last_connected_ = time;
}

auto AddressPrivate::SetServices(const Set<bitcoin::Service>& services) noexcept
    -> void
{
    services_ = services;
}

auto AddressPrivate::Subtype() const noexcept -> Transport { return subtype_; }

auto AddressPrivate::Style() const noexcept -> Protocol { return protocol_; }

auto AddressPrivate::Type() const noexcept -> Transport { return type_; }

AddressPrivate::~AddressPrivate() = default;
}  // namespace opentxs::network::blockchain

namespace opentxs::network::blockchain
{
const VersionNumber AddressPrivate::DefaultVersion{1};
}  // namespace opentxs::network::blockchain
