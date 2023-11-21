// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/FactoryPrivate.hpp"  // IWYU pragma: associated

#include <HDPath.pb.h>
#include <Identifier.pb.h>
#include <boost/endian/buffers.hpp>
#include <boost/endian/conversion.hpp>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <utility>

#include "core/identifier/IdentifierPrivate.hpp"
#include "internal/core/Core.hpp"
#include "internal/core/Factory.hpp"
#include "internal/core/String.hpp"
#include "internal/core/identifier/Factory.hpp"
#include "internal/core/identifier/Identifier.hpp"
#include "internal/network/asio/Types.hpp"
#include "internal/network/blockchain/Factory.hpp"
#include "internal/network/blockchain/Types.hpp"
#include "internal/otx/common/Cheque.hpp"
#include "internal/otx/common/Item.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Hash.hpp"  // IWYU pragma: keep
#include "opentxs/api/crypto/Util.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/core/identifier/AccountSubtype.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Algorithm.hpp"       // IWYU pragma: keep
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/HDSeed.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Type.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Types.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/crypto/HashType.hpp"  // IWYU pragma: keep
#include "opentxs/internal.factory.hpp"
#include "opentxs/network/blockchain/Address.hpp"
#include "opentxs/network/blockchain/Transport.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::api
{
template <typename IDType>
auto FactoryPrivate::id_type() noexcept -> identifier::Type
{
    return identifier::Type::invalid;
}

template <>
auto FactoryPrivate::id_type<identifier::Account>() noexcept -> identifier::Type
{
    return identifier::Type::account;
}

template <>
auto FactoryPrivate::id_type<identifier::Generic>() noexcept -> identifier::Type
{
    return identifier::Type::generic;
}

template <>
auto FactoryPrivate::id_type<identifier::Notary>() noexcept -> identifier::Type
{
    return identifier::Type::notary;
}

template <>
auto FactoryPrivate::id_type<identifier::Nym>() noexcept -> identifier::Type
{
    return identifier::Type::nym;
}

template <>
auto FactoryPrivate::id_type<identifier::HDSeed>() noexcept -> identifier::Type
{
    return identifier::Type::hdseed;
}

template <>
auto FactoryPrivate::id_type<identifier::UnitDefinition>() noexcept
    -> identifier::Type
{
    return identifier::Type::unitdefinition;
}

template <typename IDType>
auto FactoryPrivate::id_from_base58(
    const std::string_view base58,
    alloc::Default alloc) const noexcept -> IDType
{
    using namespace identifier;
    const auto& log = LogTrace();

    try {
        // NOTE empty string is a valid input
        if (false == valid(base58)) { return {}; }

        if (base58.size() < identifier_prefix_.size()) {

            throw std::runtime_error{"input too short (prefix)"};
        }

        const auto prefix = base58.substr(0_uz, identifier_prefix_.size());

        if (identifier_prefix_ != prefix) {
            const auto error = CString{"prefix (", alloc}
                                   .append(prefix)
                                   .append(") does not match expected value (")
                                   .append(identifier_prefix_)
                                   .append(")");

            throw std::runtime_error{error.c_str()};
        }

        const auto bytes = [&] {
            auto out = ByteArray{};
            const auto rc = crypto_.Encode().Base58CheckDecode(
                base58.substr(identifier_prefix_.size()), out.WriteInto());

            if (false == rc) {
                throw std::runtime_error{"base58 decode failure"};
            }

            return out;
        }();
        auto data = bytes.Bytes();
        const auto algo = [&] {
            auto value = std::byte{};
            deserialize_object(data, value, "hash algorithm");

            return deserialize_algorithm(std::to_integer<std::uint8_t>(value));
        }();
        const auto type = [&] {
            auto buf = boost::endian::little_uint16_buf_t{};
            deserialize_object(data, buf, "identifier type");

            return deserialize_identifier_type(buf.value());
        }();
        const auto expectedType = id_type<IDType>();
        const auto effectiveType = [&] {
            constexpr auto generic = Type::generic;

            if (generic == type) {

                return expectedType;
            } else {
                if (type != expectedType) {
                    log()("instantiating ")(print(type))(" identifier as ")(
                        print(expectedType))
                        .Flush();
                }

                return type;
            }
        }();
        const auto hash = [&]() -> ReadView {
            if (data.empty()) {

                return {};
            } else {
                const auto hashBytes = identifier_expected_hash_bytes(algo);

                return extract_prefix(data, hashBytes, "hash");
            }
        }();
        const auto accountSubtype = [&] {
            if (data.empty() || (Type::account != effectiveType)) {

                return AccountSubtype::invalid_subtype;
            } else {
                auto buf = boost::endian::little_uint16_buf_t{};
                deserialize_object(data, buf, "account subtype");

                return deserialize_account_subtype(buf.value());
            }
        }();

        return factory::Identifier(
            effectiveType, algo, hash, accountSubtype, std::move(alloc));
    } catch (const std::exception& e) {
        log()(e.what()).Flush();

        return factory::IdentifierInvalid(std::move(alloc));
    }
}

template <typename IDType>
auto FactoryPrivate::id_from_hash(
    const ReadView bytes,
    const identifier::Algorithm type,
    alloc::Default alloc) const noexcept -> IDType
{
    return id_from_hash<IDType>(
        bytes, type, identifier::AccountSubtype::invalid_subtype, alloc);
}

template <typename IDType>
auto FactoryPrivate::id_from_hash(
    const ReadView bytes,
    const identifier::Algorithm type,
    identifier::AccountSubtype subtype,
    alloc::Default alloc) const noexcept -> IDType
{
    auto out = IDType{factory::Identifier(id_type<IDType>(), subtype, alloc)};
    const auto expected = identifier_expected_hash_bytes(type);

    if (const auto size = bytes.size(); size == expected) {
        out.Assign(bytes);
    } else {
        LogError()()("expected ")(bytes)(" bytes but supplied hash is ")(
            size)(" bytes")
            .Flush();
    }

    return out;
}

template <typename IDType>
auto FactoryPrivate::id_from_preimage(
    const identifier::Algorithm type,
    const ReadView preimage,
    alloc::Default alloc) const noexcept -> IDType
{
    return id_from_preimage<IDType>(
        type, identifier::AccountSubtype::invalid_subtype, preimage, alloc);
}

template <typename IDType>
auto FactoryPrivate::id_from_preimage(
    const identifier::Algorithm type,
    identifier::AccountSubtype subtype,
    const ReadView preimage,
    alloc::Default alloc) const noexcept -> IDType
{
    try {
        const auto hashType = identifier::get_hash_type(type);
        auto out =
            IDType{factory::Identifier(id_type<IDType>(), subtype, alloc)};
        const auto rc =
            crypto_.Hash().Digest(hashType, preimage, out.WriteInto());

        if (false == rc) {

            throw std::runtime_error("failed to calculate digest");
        }

        return out;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return factory::IdentifierInvalid(alloc);
    }
}

template <typename IDType>
auto FactoryPrivate::id_from_preimage(
    const identifier::Algorithm type,
    const ProtobufType& proto,
    alloc::Default alloc) const noexcept -> IDType
{
    try {
        const auto serialized = [&] {
            auto out = ByteArray{alloc};

            if (false == proto::write(proto, out.WriteInto())) {
                throw std::runtime_error{"failed to serialize protobuf"};
            }

            return out;
        }();

        return id_from_preimage<IDType>(
            type, serialized.Bytes(), std::move(alloc));
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return factory::IdentifierInvalid(alloc);
    }
}

template <typename IDType>
auto FactoryPrivate::id_from_protobuf(
    const proto::Identifier& proto,
    alloc::Default alloc) const noexcept -> IDType
{
    using namespace identifier;

    try {
        const auto expectedType = id_type<IDType>();
        const auto type = deserialize_identifier_type(proto.type());
        const auto algo = deserialize_algorithm(proto.algorithm());
        constexpr auto generic = Type::generic;

        if ((expectedType != generic) && (type != expectedType)) {
            const auto error = CString{"serialized type (", alloc}
                                   .append(print(type))
                                   .append(") does not match expected type (")
                                   .append(print(expectedType))
                                   .append(")");

            throw std::runtime_error{error.c_str()};
        }

        const auto& hash = proto.hash();
        const auto validSize =
            (hash.empty()) ||
            (hash.size() == identifier_expected_hash_bytes(algo));

        if (false == validSize) { throw std::runtime_error{"wrong hash size"}; }

        return factory::Identifier(
            type,
            algo,
            hash,
            deserialize_account_subtype(proto.account_subtype()),
            std::move(alloc));
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return factory::IdentifierInvalid(std::move(alloc));
    }
}

template <typename IDType>
auto FactoryPrivate::id_from_random(
    const identifier::Algorithm type,
    alloc::Default alloc) const noexcept -> IDType
{
    return id_from_random<IDType>(
        type, identifier::AccountSubtype::invalid_subtype, alloc);
}

template <typename IDType>
auto FactoryPrivate::id_from_random(
    const identifier::Algorithm type,
    identifier::AccountSubtype subtype,
    alloc::Default alloc) const noexcept -> IDType
{
    try {
        auto out =
            IDType{factory::Identifier(id_type<IDType>(), subtype, alloc)};
        const auto size = identifier_expected_hash_bytes(type);

        if (0_uz == size) { throw std::runtime_error{"invalid hash type"}; }

        if (false == out.resize(size)) {
            throw std::runtime_error{"failed to reserve space for hash"};
        }

        assert_true(out.size() == size);

        if (false == crypto_.Util().RandomizeMemory(out.data(), out.size())) {
            throw std::runtime_error{"failed to randomize hash"};
        }

        return out;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::api

namespace opentxs::api
{
FactoryPrivate::FactoryPrivate(const api::Crypto& crypto) noexcept
    : crypto_(crypto)
    , self_(this)
{
}

auto FactoryPrivate::AccountID(
    const identity::wot::claim::ClaimType type,
    const proto::HDPath& path,
    alloc::Default alloc) const noexcept -> identifier::Account
{
    const auto preimage = [&] {
        auto out = ByteArray{};
        proto::write(path, out.WriteInto());
        auto sType = static_cast<std::uint32_t>(type);
        boost::endian::native_to_little_inplace(sType);
        out.Concatenate(std::addressof(sType), sizeof(sType));

        return out;
    }();
    using enum identifier::AccountSubtype;

    return AccountIDFromPreimage(
        preimage.Bytes(), blockchain_subaccount, std::move(alloc));
}

auto FactoryPrivate::AccountID(
    const proto::Identifier& in,
    alloc::Default alloc) const noexcept -> identifier::Account
{
    return id_from_protobuf<identifier::Account>(in, std::move(alloc));
}

auto FactoryPrivate::AccountID(const Contract& contract, alloc::Default alloc)
    const noexcept -> identifier::Account
{
    const auto preimage = String::Factory(contract);
    using enum identifier::AccountSubtype;

    return AccountIDFromPreimage(
        preimage->Bytes(), custodial_account, std::move(alloc));
}

auto FactoryPrivate::AccountIDConvertSafe(
    const identifier::Generic& in,
    alloc::Default alloc) const noexcept -> identifier::Account
{
    using enum identifier::Type;

    switch (in.Type()) {
        case account:
        case generic: {

            return factory::Identifier(
                account,
                in.Algorithm(),
                in.Bytes(),
                in.Internal().Get().account_subtype_,
                std::move(alloc));
        }
        case invalid:
        case nym:
        case notary:
        case unitdefinition:
        case hdseed:
        default: {

            return factory::IdentifierInvalid(std::move(alloc));
        }
    }
}

auto FactoryPrivate::AccountIDFromBase58(
    const std::string_view base58,
    alloc::Default alloc) const noexcept -> identifier::Account
{
    return id_from_base58<identifier::Account>(base58, std::move(alloc));
}

auto FactoryPrivate::AccountIDFromHash(
    const ReadView bytes,
    identifier::AccountSubtype subtype,
    alloc::Default alloc) const noexcept -> identifier::Account
{
    return AccountIDFromHash(
        bytes, subtype, default_identifier_algorithm(), std::move(alloc));
}

auto FactoryPrivate::AccountIDFromHash(
    const ReadView bytes,
    identifier::AccountSubtype subtype,
    const identifier::Algorithm type,
    alloc::Default alloc) const noexcept -> identifier::Account
{
    return id_from_hash<identifier::Account>(
        bytes, type, subtype, std::move(alloc));
}

auto FactoryPrivate::AccountIDFromPreimage(
    const ReadView preimage,
    identifier::AccountSubtype subtype,
    alloc::Default alloc) const noexcept -> identifier::Account
{
    return AccountIDFromPreimage(
        preimage, subtype, default_identifier_algorithm(), std::move(alloc));
}

auto FactoryPrivate::AccountIDFromPreimage(
    const ReadView preimage,
    identifier::AccountSubtype subtype,
    const identifier::Algorithm type,
    alloc::Default alloc) const noexcept -> identifier::Account
{
    return id_from_preimage<identifier::Account>(
        type, subtype, preimage, std::move(alloc));
}

auto FactoryPrivate::AccountIDFromProtobuf(
    const ReadView bytes,
    alloc::Default alloc) const noexcept -> identifier::Account
{
    return AccountID(proto::Factory<proto::Identifier>(bytes), alloc);
}

auto FactoryPrivate::AccountIDFromRandom(
    identifier::AccountSubtype subtype,
    alloc::Default alloc) const noexcept -> identifier::Account
{
    return AccountIDFromRandom(
        subtype, default_identifier_algorithm(), std::move(alloc));
}

auto FactoryPrivate::AccountIDFromRandom(
    identifier::AccountSubtype subtype,
    const identifier::Algorithm type,
    alloc::Default alloc) const noexcept -> identifier::Account
{
    return id_from_random<identifier::Account>(type, subtype, std::move(alloc));
}

auto FactoryPrivate::AccountIDFromZMQ(
    const opentxs::network::zeromq::Frame& in,
    alloc::Default alloc) const noexcept -> identifier::Account
{
    return AccountIDFromZMQ(in.Bytes(), alloc);
}

auto FactoryPrivate::AccountIDFromZMQ(
    const ReadView frame,
    alloc::Default alloc) const noexcept -> identifier::Account
{
    return AccountID(proto::Factory<proto::Identifier>(frame), alloc);
}

auto FactoryPrivate::Amount(const opentxs::network::zeromq::Frame& zmq)
    const noexcept -> opentxs::Amount
{
    return factory::Amount(zmq);
}

auto FactoryPrivate::Armored() const -> OTArmored
{
    return OTArmored{opentxs::Factory::Armored(crypto_)};
}

auto FactoryPrivate::Armored(const UnallocatedCString& input) const -> OTArmored
{
    return OTArmored{
        opentxs::Factory::Armored(crypto_, String::Factory(input.c_str()))};
}

auto FactoryPrivate::Armored(const opentxs::Data& input) const -> OTArmored
{
    return OTArmored{opentxs::Factory::Armored(crypto_, input)};
}

auto FactoryPrivate::Armored(const opentxs::String& input) const -> OTArmored
{
    return OTArmored{opentxs::Factory::Armored(crypto_, input)};
}

auto FactoryPrivate::Armored(const opentxs::crypto::Envelope& input) const
    -> OTArmored
{
    return OTArmored{opentxs::Factory::Armored(crypto_, input)};
}

auto FactoryPrivate::Armored(const ProtobufType& input) const -> OTArmored
{
    return OTArmored{opentxs::Factory::Armored(crypto_, Data(input))};
}

auto FactoryPrivate::Armored(
    const ProtobufType& input,
    const UnallocatedCString& header) const -> OTString
{
    auto armored = Armored(Data(input));
    auto output = String::Factory();
    armored->WriteArmoredString(output, header);

    return output;
}

auto FactoryPrivate::BlockchainAddress(
    const opentxs::network::blockchain::Protocol protocol,
    const opentxs::network::blockchain::Transport network,
    const ReadView bytes,
    const std::uint16_t port,
    const opentxs::blockchain::Type chain,
    const Time lastConnected,
    const Set<opentxs::network::blockchain::bitcoin::Service>& services)
    const noexcept -> opentxs::network::blockchain::Address
{
    using enum opentxs::network::blockchain::Transport;

    return factory::BlockchainAddress(
        crypto_,
        Self(),
        protocol,
        network,
        invalid,
        bytes,
        port,
        chain,
        lastConnected,
        services,
        false,
        {});
}

auto FactoryPrivate::BlockchainAddress(
    const opentxs::network::blockchain::Protocol protocol,
    const boost::asio::ip::address& address,
    const std::uint16_t port,
    const blockchain::Type chain,
    const Time lastConnected,
    const Set<opentxs::network::blockchain::bitcoin::Service>& services)
    const noexcept -> opentxs::network::blockchain::Address
{
    return BlockchainAddress(
        protocol,
        opentxs::network::asio::type(address),
        opentxs::network::asio::serialize(address).Bytes(),
        port,
        chain,
        lastConnected,
        services);
}

auto FactoryPrivate::BlockchainAddress(
    const proto::BlockchainPeerAddress& serialized) const noexcept
    -> opentxs::network::blockchain::Address
{
    return factory::BlockchainAddress(crypto_, Self(), serialized);
}

auto FactoryPrivate::BlockchainAddressIncoming(
    const opentxs::network::blockchain::Protocol protocol,
    const opentxs::network::blockchain::Transport network,
    const opentxs::network::blockchain::Transport subtype,
    const ReadView bytes,
    const std::uint16_t port,
    const opentxs::blockchain::Type chain,
    const Time lastConnected,
    const Set<opentxs::network::blockchain::bitcoin::Service>& services,
    const ReadView cookie) const noexcept
    -> opentxs::network::blockchain::Address
{
    return factory::BlockchainAddress(
        crypto_,
        Self(),
        protocol,
        network,
        subtype,
        bytes,
        port,
        chain,
        lastConnected,
        services,
        true,
        cookie);
}

auto FactoryPrivate::BlockchainAddressZMQ(
    const opentxs::network::blockchain::Protocol protocol,
    const opentxs::network::blockchain::Transport network,
    const ReadView bytes,
    const blockchain::Type chain,
    const Time lastConnected,
    const Set<opentxs::network::blockchain::bitcoin::Service>& services,
    const ReadView key) const noexcept -> opentxs::network::blockchain::Address
{
    using enum opentxs::network::blockchain::Transport;
    auto decoded = std::optional<ByteArray>{};
    const auto keyBytes = [&] {
        if (const auto size = key.size(); 0_uz == size % 4) {

            return key;
        } else if (0_uz == size % 5) {
            auto& buf = decoded.emplace();

            if (false == crypto_.Encode().Z85Decode(key, buf.WriteInto())) {
                LogError()()("unable to decode key as Z85: ")
                    .asHex(key)
                    .Flush();
            }

            return buf.Bytes();
        } else {
            LogError()()("invalid key length: ")(size).Flush();

            return ReadView{};
        }
    }();

    if (keyBytes.empty()) {

        return {};
    } else {

        return factory::BlockchainAddress(
            crypto_,
            Self(),
            protocol,
            zmq,
            network,
            keyBytes,
            bytes,
            opentxs::network::blockchain::otdht_listen_port_,
            chain,
            lastConnected,
            services,
            false,
            {});
    }
}

auto FactoryPrivate::BlockchainAddressZMQ(
    const opentxs::network::blockchain::Protocol protocol,
    const boost::asio::ip::address& address,
    const blockchain::Type chain,
    const Time lastConnected,
    const Set<opentxs::network::blockchain::bitcoin::Service>& services,
    const ReadView key) const noexcept -> opentxs::network::blockchain::Address
{
    return BlockchainAddressZMQ(
        protocol,
        opentxs::network::asio::type(address),
        opentxs::network::asio::serialize(address).Bytes(),
        chain,
        lastConnected,
        services,
        key);
}

auto FactoryPrivate::Data() const -> ByteArray { return {}; }

auto FactoryPrivate::Data(const opentxs::Armored& input) const -> ByteArray
{
    return input;
}

auto FactoryPrivate::Data(const opentxs::network::zeromq::Frame& input) const
    -> ByteArray
{
    return input.Bytes();
}

auto FactoryPrivate::Data(const std::uint8_t input) const -> ByteArray
{
    return input;
}

auto FactoryPrivate::Data(const std::uint32_t input) const -> ByteArray
{
    return input;
}

auto FactoryPrivate::Data(const UnallocatedVector<unsigned char>& input) const
    -> ByteArray
{
    return {input.data(), input.size()};
}

auto FactoryPrivate::Data(const UnallocatedVector<std::byte>& input) const
    -> ByteArray
{
    return {input.data(), input.size()};
}

auto FactoryPrivate::DataFromBytes(ReadView input) const -> ByteArray
{
    return input;
}

auto FactoryPrivate::DataFromHex(ReadView input) const -> ByteArray
{
    try {

        return {IsHex, input};
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return {};
    }
}

auto FactoryPrivate::Data(const ProtobufType& input) const -> ByteArray
{
    auto output = ByteArray{};
    const auto size{input.ByteSize()};
    output.resize(size);
    input.SerializeToArray(output.data(), size);

    return output;
}

auto FactoryPrivate::Identifier(const Cheque& cheque, alloc::Default alloc)
    const noexcept -> identifier::Generic
{
    const auto preimage = String::Factory(cheque);

    return IdentifierFromPreimage(preimage->Bytes(), std::move(alloc));
}

auto FactoryPrivate::Identifier(const Contract& contract, alloc::Default alloc)
    const noexcept -> identifier::Generic
{
    const auto preimage = String::Factory(contract);

    return IdentifierFromPreimage(preimage->Bytes(), std::move(alloc));
}

auto FactoryPrivate::Identifier(const Item& item, alloc::Default alloc)
    const noexcept -> identifier::Generic
{
    const auto preimage = String::Factory(item);

    return IdentifierFromPreimage(preimage->Bytes(), std::move(alloc));
}

auto FactoryPrivate::Identifier(
    const proto::Identifier& in,
    alloc::Default alloc) const noexcept -> identifier::Generic
{
    return id_from_protobuf<identifier::Generic>(in, std::move(alloc));
}

auto FactoryPrivate::IdentifierFromBase58(
    const std::string_view base58,
    alloc::Default alloc) const noexcept -> identifier::Generic
{
    return id_from_base58<identifier::Generic>(base58, std::move(alloc));
}

auto FactoryPrivate::IdentifierFromHash(
    const ReadView bytes,
    alloc::Default alloc) const noexcept -> identifier::Generic
{
    return IdentifierFromHash(
        bytes, default_identifier_algorithm(), std::move(alloc));
}

auto FactoryPrivate::IdentifierFromHash(
    const ReadView bytes,
    const identifier::Algorithm type,
    alloc::Default alloc) const noexcept -> identifier::Generic
{
    return id_from_hash<identifier::Generic>(bytes, type, std::move(alloc));
}

auto FactoryPrivate::IdentifierFromPreimage(
    const ReadView preimage,
    alloc::Default alloc) const noexcept -> identifier::Generic
{
    return IdentifierFromPreimage(
        preimage, default_identifier_algorithm(), std::move(alloc));
}

auto FactoryPrivate::IdentifierFromPreimage(
    const ReadView preimage,
    const identifier::Algorithm type,
    alloc::Default alloc) const noexcept -> identifier::Generic
{
    return id_from_preimage<identifier::Generic>(
        type, preimage, std::move(alloc));
}

auto FactoryPrivate::IdentifierFromPreimage(
    const ProtobufType& proto,
    alloc::Default alloc) const noexcept -> identifier::Generic
{
    return IdentifierFromPreimage(
        proto, default_identifier_algorithm(), std::move(alloc));
}

auto FactoryPrivate::IdentifierFromPreimage(
    const ProtobufType& proto,
    const identifier::Algorithm type,
    alloc::Default alloc) const noexcept -> identifier::Generic
{
    return id_from_preimage<identifier::Generic>(type, proto, std::move(alloc));
}

auto FactoryPrivate::IdentifierFromProtobuf(
    const ReadView bytes,
    alloc::Default alloc) const noexcept -> identifier::Generic
{
    return Identifier(proto::Factory<proto::Identifier>(bytes), alloc);
}

auto FactoryPrivate::IdentifierFromRandom(alloc::Default alloc) const noexcept
    -> identifier::Generic
{
    return IdentifierFromRandom(
        default_identifier_algorithm(), std::move(alloc));
}

auto FactoryPrivate::IdentifierFromRandom(
    const identifier::Algorithm type,
    alloc::Default alloc) const noexcept -> identifier::Generic
{
    return id_from_random<identifier::Generic>(type, std::move(alloc));
}

auto FactoryPrivate::NotaryID(const proto::Identifier& in, alloc::Default alloc)
    const noexcept -> identifier::Notary
{
    return id_from_protobuf<identifier::Notary>(in, std::move(alloc));
}

auto FactoryPrivate::NotaryIDConvertSafe(
    const identifier::Generic& in,
    alloc::Default alloc) const noexcept -> identifier::Notary
{
    using enum identifier::Type;

    switch (in.Type()) {
        case notary:
        case generic: {

            return factory::Identifier(
                notary,
                in.Algorithm(),
                in.Bytes(),
                identifier::AccountSubtype::invalid_subtype,
                std::move(alloc));
        }
        case invalid:
        case nym:
        case unitdefinition:
        case account:
        case hdseed:
        default: {

            return factory::IdentifierInvalid(std::move(alloc));
        }
    }
}

auto FactoryPrivate::NotaryIDFromBase58(
    const std::string_view base58,
    alloc::Default alloc) const noexcept -> identifier::Notary
{
    return id_from_base58<identifier::Notary>(base58, std::move(alloc));
}

auto FactoryPrivate::NotaryIDFromHash(
    const ReadView bytes,
    alloc::Default alloc) const noexcept -> identifier::Notary
{
    return NotaryIDFromHash(
        bytes, default_identifier_algorithm(), std::move(alloc));
}

auto FactoryPrivate::NotaryIDFromHash(
    const ReadView bytes,
    const identifier::Algorithm type,
    alloc::Default alloc) const noexcept -> identifier::Notary
{
    return id_from_hash<identifier::Notary>(bytes, type, std::move(alloc));
}

auto FactoryPrivate::NotaryIDFromPreimage(
    const ReadView preimage,
    alloc::Default alloc) const noexcept -> identifier::Notary
{
    return NotaryIDFromPreimage(
        preimage, default_identifier_algorithm(), std::move(alloc));
}

auto FactoryPrivate::NotaryIDFromPreimage(
    const ReadView preimage,
    const identifier::Algorithm type,
    alloc::Default alloc) const noexcept -> identifier::Notary
{
    return id_from_preimage<identifier::Notary>(
        type, preimage, std::move(alloc));
}

auto FactoryPrivate::NotaryIDFromPreimage(
    const ProtobufType& proto,
    alloc::Default alloc) const noexcept -> identifier::Notary
{
    return NotaryIDFromPreimage(
        proto, default_identifier_algorithm(), std::move(alloc));
}

auto FactoryPrivate::NotaryIDFromPreimage(
    const ProtobufType& proto,
    const identifier::Algorithm type,
    alloc::Default alloc) const noexcept -> identifier::Notary
{
    return id_from_preimage<identifier::Notary>(type, proto, std::move(alloc));
}

auto FactoryPrivate::NotaryIDFromProtobuf(
    const ReadView bytes,
    alloc::Default alloc) const noexcept -> identifier::Notary
{
    return NotaryID(proto::Factory<proto::Identifier>(bytes), alloc);
}

auto FactoryPrivate::NotaryIDFromRandom(alloc::Default alloc) const noexcept
    -> identifier::Notary
{
    return NotaryIDFromRandom(default_identifier_algorithm(), std::move(alloc));
}

auto FactoryPrivate::NotaryIDFromRandom(
    const identifier::Algorithm type,
    alloc::Default alloc) const noexcept -> identifier::Notary
{
    return id_from_random<identifier::Notary>(type, std::move(alloc));
}

auto FactoryPrivate::NymID(const proto::Identifier& in, alloc::Default alloc)
    const noexcept -> identifier::Nym
{
    return id_from_protobuf<identifier::Nym>(in, std::move(alloc));
}

auto FactoryPrivate::NymIDConvertSafe(
    const identifier::Generic& in,
    alloc::Default alloc) const noexcept -> identifier::Nym
{
    using enum identifier::Type;

    switch (in.Type()) {
        case nym:
        case generic: {

            return factory::Identifier(
                nym,
                in.Algorithm(),
                in.Bytes(),
                identifier::AccountSubtype::invalid_subtype,
                std::move(alloc));
        }
        case invalid:
        case unitdefinition:
        case notary:
        case account:
        case hdseed:
        default: {

            return factory::IdentifierInvalid(std::move(alloc));
        }
    }
}

auto FactoryPrivate::NymIDFromBase58(
    const std::string_view base58,
    alloc::Default alloc) const noexcept -> identifier::Nym
{
    return id_from_base58<identifier::Nym>(base58, std::move(alloc));
}

auto FactoryPrivate::NymIDFromHash(const ReadView bytes, alloc::Default alloc)
    const noexcept -> identifier::Nym
{
    return NymIDFromHash(
        bytes, default_identifier_algorithm(), std::move(alloc));
}

auto FactoryPrivate::NymIDFromHash(
    const ReadView bytes,
    const identifier::Algorithm type,
    alloc::Default alloc) const noexcept -> identifier::Nym
{
    return id_from_hash<identifier::Nym>(bytes, type, std::move(alloc));
}

auto FactoryPrivate::NymIDFromPreimage(
    const ReadView preimage,
    alloc::Default alloc) const noexcept -> identifier::Nym
{
    return NymIDFromPreimage(
        preimage, default_identifier_algorithm(), std::move(alloc));
}

auto FactoryPrivate::NymIDFromPreimage(
    const ReadView preimage,
    const identifier::Algorithm type,
    alloc::Default alloc) const noexcept -> identifier::Nym
{
    return id_from_preimage<identifier::Nym>(type, preimage, std::move(alloc));
}

auto FactoryPrivate::NymIDFromProtobuf(
    const ReadView bytes,
    alloc::Default alloc) const noexcept -> identifier::Nym
{
    return NymID(proto::Factory<proto::Identifier>(bytes), alloc);
}

auto FactoryPrivate::NymIDFromRandom(alloc::Default alloc) const noexcept
    -> identifier::Nym
{
    return NymIDFromRandom(default_identifier_algorithm(), std::move(alloc));
}

auto FactoryPrivate::NymIDFromRandom(
    const identifier::Algorithm type,
    alloc::Default alloc) const noexcept -> identifier::Nym
{
    return id_from_random<identifier::Nym>(type, std::move(alloc));
}

auto FactoryPrivate::Secret(const std::size_t bytes) const noexcept
    -> opentxs::Secret
{
    return factory::Secret(bytes);
}

auto FactoryPrivate::SecretFromBytes(const ReadView bytes) const noexcept
    -> opentxs::Secret
{
    return factory::Secret(bytes, true);
}

auto FactoryPrivate::SecretFromText(const std::string_view text) const noexcept
    -> opentxs::Secret
{
    return factory::Secret(text, false);
}

auto FactoryPrivate::SeedID(const proto::Identifier& in, alloc::Default alloc)
    const noexcept -> identifier::HDSeed
{
    return id_from_protobuf<identifier::HDSeed>(in, std::move(alloc));
}

auto FactoryPrivate::SeedIDFromBase58(
    const std::string_view base58,
    alloc::Default alloc) const noexcept -> identifier::HDSeed
{
    return id_from_base58<identifier::HDSeed>(base58, std::move(alloc));
}

auto FactoryPrivate::SeedIDFromHash(const ReadView bytes, alloc::Default alloc)
    const noexcept -> identifier::HDSeed
{
    return SeedIDFromHash(
        bytes, default_identifier_algorithm(), std::move(alloc));
}

auto FactoryPrivate::SeedIDFromHash(
    const ReadView bytes,
    const identifier::Algorithm type,
    alloc::Default alloc) const noexcept -> identifier::HDSeed
{
    return id_from_hash<identifier::HDSeed>(bytes, type, std::move(alloc));
}

auto FactoryPrivate::SeedIDFromPreimage(
    const ReadView preimage,
    alloc::Default alloc) const noexcept -> identifier::HDSeed
{
    return SeedIDFromPreimage(
        preimage, default_identifier_algorithm(), std::move(alloc));
}

auto FactoryPrivate::SeedIDFromPreimage(
    const ReadView preimage,
    const identifier::Algorithm type,
    alloc::Default alloc) const noexcept -> identifier::HDSeed
{
    return id_from_preimage<identifier::HDSeed>(
        type, preimage, std::move(alloc));
}

auto FactoryPrivate::SeedIDFromProtobuf(
    const ReadView bytes,
    alloc::Default alloc) const noexcept -> identifier::HDSeed
{
    return SeedID(proto::Factory<proto::Identifier>(bytes), alloc);
}

auto FactoryPrivate::SeedIDFromRandom(alloc::Default alloc) const noexcept
    -> identifier::HDSeed
{
    return SeedIDFromRandom(default_identifier_algorithm(), std::move(alloc));
}

auto FactoryPrivate::SeedIDFromRandom(
    const identifier::Algorithm type,
    alloc::Default alloc) const noexcept -> identifier::HDSeed
{
    return id_from_random<identifier::HDSeed>(type, std::move(alloc));
}

auto FactoryPrivate::Session() const noexcept
    -> const api::session::internal::Factory&
{
    return const_cast<FactoryPrivate*>(this)->Session();
}

auto FactoryPrivate::Session() noexcept -> api::session::internal::Factory&
{
    LogAbort()()("not a session instance").Abort();
}

auto FactoryPrivate::UnitID(const proto::Identifier& in, alloc::Default alloc)
    const noexcept -> identifier::UnitDefinition
{
    return id_from_protobuf<identifier::UnitDefinition>(in, std::move(alloc));
}

auto FactoryPrivate::UnitIDConvertSafe(
    const identifier::Generic& in,
    alloc::Default alloc) const noexcept -> identifier::UnitDefinition
{
    using enum identifier::Type;

    switch (in.Type()) {
        case unitdefinition:
        case generic: {

            return factory::Identifier(
                unitdefinition,
                in.Algorithm(),
                in.Bytes(),
                identifier::AccountSubtype::invalid_subtype,
                std::move(alloc));
        }
        case invalid:
        case nym:
        case notary:
        case account:
        case hdseed:
        default: {

            return factory::IdentifierInvalid(std::move(alloc));
        }
    }
}

auto FactoryPrivate::UnitIDFromBase58(
    const std::string_view base58,
    alloc::Default alloc) const noexcept -> identifier::UnitDefinition
{
    return id_from_base58<identifier::UnitDefinition>(base58, std::move(alloc));
}

auto FactoryPrivate::UnitIDFromHash(const ReadView bytes, alloc::Default alloc)
    const noexcept -> identifier::UnitDefinition
{
    return UnitIDFromHash(
        bytes, default_identifier_algorithm(), std::move(alloc));
}

auto FactoryPrivate::UnitIDFromHash(
    const ReadView bytes,
    const identifier::Algorithm type,
    alloc::Default alloc) const noexcept -> identifier::UnitDefinition
{
    return id_from_hash<identifier::UnitDefinition>(
        bytes, type, std::move(alloc));
}

auto FactoryPrivate::UnitIDFromPreimage(
    const ReadView preimage,
    alloc::Default alloc) const noexcept -> identifier::UnitDefinition
{
    return UnitIDFromPreimage(
        preimage, default_identifier_algorithm(), std::move(alloc));
}

auto FactoryPrivate::UnitIDFromPreimage(
    const ReadView preimage,
    const identifier::Algorithm type,
    alloc::Default alloc) const noexcept -> identifier::UnitDefinition
{
    return id_from_preimage<identifier::UnitDefinition>(
        type, preimage, std::move(alloc));
}

auto FactoryPrivate::UnitIDFromPreimage(
    const ProtobufType& proto,
    alloc::Default alloc) const noexcept -> identifier::UnitDefinition
{
    return UnitIDFromPreimage(
        proto, default_identifier_algorithm(), std::move(alloc));
}

auto FactoryPrivate::UnitIDFromPreimage(
    const ProtobufType& proto,
    const identifier::Algorithm type,
    alloc::Default alloc) const noexcept -> identifier::UnitDefinition
{
    return id_from_preimage<identifier::UnitDefinition>(
        type, proto, std::move(alloc));
}

auto FactoryPrivate::UnitIDFromProtobuf(
    const ReadView bytes,
    alloc::Default alloc) const noexcept -> identifier::UnitDefinition
{
    return UnitID(proto::Factory<proto::Identifier>(bytes), alloc);
}

auto FactoryPrivate::UnitIDFromRandom(alloc::Default alloc) const noexcept
    -> identifier::UnitDefinition
{
    return UnitIDFromRandom(default_identifier_algorithm(), std::move(alloc));
}

auto FactoryPrivate::UnitIDFromRandom(
    const identifier::Algorithm type,
    alloc::Default alloc) const noexcept -> identifier::UnitDefinition
{
    return id_from_random<identifier::UnitDefinition>(type, std::move(alloc));
}

FactoryPrivate::~FactoryPrivate() { Detach(self_); }
}  // namespace opentxs::api
