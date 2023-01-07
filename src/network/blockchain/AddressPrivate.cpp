// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/Factory.hpp"  // IWYU pragma: associated
#include "network/blockchain/AddressPrivate.hpp"    // IWYU pragma: associated

#include <BlockchainPeerAddress.pb.h>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>

#include "BoostAsio.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/network/asio/Types.hpp"
#include "internal/network/blockchain/bitcoin/message/Types.hpp"  // IWYU pragma: keep
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Time.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/network/blockchain/Address.hpp"
#include "opentxs/network/blockchain/Transport.hpp"  // IWYU pragma: keep
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::network::blockchain::implementation
{
class Address final : public AddressPrivate
{
public:
    static const VersionNumber DefaultVersion;

    static auto instantiate_services(
        const proto::BlockchainPeerAddress& serialized) noexcept
        -> Set<bitcoin::Service>
    {
        auto output = Set<bitcoin::Service>{};

        for (const auto& service : serialized.service()) {
            output.emplace(static_cast<bitcoin::Service>(service));
        }

        return output;
    }

    auto Bytes() const noexcept -> ByteArray final { return bytes_; }
    auto Chain() const noexcept -> opentxs::blockchain::Type final
    {
        return chain_;
    }
    auto clone() const noexcept -> std::unique_ptr<AddressPrivate> final
    {
        return std::make_unique<Address>(*this);
    }
    auto Cookie() const noexcept -> ReadView final { return cookie_.Bytes(); }
    auto Display() const noexcept -> UnallocatedCString final
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
            output =
                UnallocatedCString(
                    static_cast<const char*>(bytes_.data()), bytes_.size()) +
                ".onion";
        };
        const auto printEep = [&]() {
            // TODO handle errors
            [[maybe_unused]] const auto rc =
                api_.Crypto().Encode().Base64Encode(
                    bytes_.Bytes(), writer(output));
            output += ".i2p";
        };
        using enum Transport;

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

        return output + ":" + std::to_string(port_);
    }
    auto ID() const noexcept -> const identifier::Generic& final { return id_; }
    auto Incoming() const noexcept -> bool final { return incoming_; }
    auto IsValid() const noexcept -> bool final { return true; }
    auto Key() const noexcept -> ReadView final { return key_.Bytes(); }
    auto LastConnected() const noexcept -> Time final
    {
        return last_connected_;
    }
    auto Port() const noexcept -> std::uint16_t final { return port_; }
    auto PreviousLastConnected() const noexcept -> Time final
    {
        return previous_last_connected_;
    }
    auto PreviousServices() const noexcept -> Set<bitcoin::Service> final
    {
        return previous_services_;
    }
    auto Serialize(proto::BlockchainPeerAddress& out) const noexcept
        -> bool final
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
        out.set_id(id_.asBase58(api_.Crypto()));

        return true;
    }
    auto Services() const noexcept -> Set<bitcoin::Service> final
    {
        return services_;
    }
    auto Subtype() const noexcept -> Transport final { return subtype_; }
    auto Style() const noexcept -> Protocol final { return protocol_; }
    auto Type() const noexcept -> Transport final { return type_; }

    auto AddService(const bitcoin::Service service) noexcept -> void final
    {
        services_.emplace(service);
    }
    auto RemoveService(const bitcoin::Service service) noexcept -> void final
    {
        services_.erase(service);
    }
    auto SetIncoming(bool value) noexcept -> void final { incoming_ = value; }
    auto SetLastConnected(const Time& time) noexcept -> void final
    {
        last_connected_ = time;
    }
    auto SetServices(const Set<bitcoin::Service>& services) noexcept
        -> void final
    {
        services_ = services;
    }

    Address(
        const api::Session& api,
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
        : api_(api)
        , version_(version)
        , id_(calculate_id(
              api,
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
                    const auto error =
                        UnallocatedCString{"expected "}
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
                    const auto error =
                        UnallocatedCString{"expected "}
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
                    const auto error =
                        UnallocatedCString{"expected "}
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
                LogAbort()(OT_PRETTY_CLASS())("unhandled transport type ")(
                    print(type_))
                    .Abort();
            }
        }
    }

    Address() = delete;
    Address(const Address& rhs) noexcept
        : api_(rhs.api_)
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
    Address(Address&&) = delete;
    auto operator=(const Address&) -> Address& = delete;
    auto operator=(Address&&) -> Address& = delete;

    ~Address() final = default;

private:
    const api::Session& api_;
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
        const api::Session& api,
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

        return api.Factory().InternalSession().IdentifierFromPreimage(
            serialized);
    }
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

        output.set_subtype(static_cast<std::uint8_t>(subtype));
        output.set_key(key.data(), key.size());

        return output;
    }
};

const VersionNumber Address::DefaultVersion{1};
}  // namespace opentxs::network::blockchain::implementation

namespace opentxs::factory
{
auto BlockchainAddress(
    const api::Session& api,
    const network::blockchain::Protocol protocol,
    const network::blockchain::Transport network,
    const ReadView bytes,
    const std::uint16_t port,
    const opentxs::blockchain::Type chain,
    const Time lastConnected,
    const Set<network::blockchain::bitcoin::Service>& services,
    const bool incoming,
    const ReadView cookie) noexcept -> network::blockchain::Address
{
    using ReturnType = network::blockchain::implementation::Address;
    using enum network::blockchain::Transport;

    try {
        return std::make_unique<ReturnType>(
                   api,
                   ReturnType::DefaultVersion,
                   protocol,
                   network,
                   invalid,
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
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto BlockchainAddress(
    const api::Session& api,
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
    using ReturnType = network::blockchain::implementation::Address;

    try {
        return std::make_unique<ReturnType>(
                   api,
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
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto BlockchainAddress(
    const api::Session& api,
    const proto::BlockchainPeerAddress& serialized) noexcept
    -> network::blockchain::Address
{
    using ReturnType = network::blockchain::implementation::Address;

    try {
        return std::make_unique<ReturnType>(
                   api,
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
                   convert_stime(serialized.time()),
                   ReturnType::instantiate_services(serialized),
                   false,
                   ReadView{})
            .release();
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::factory
