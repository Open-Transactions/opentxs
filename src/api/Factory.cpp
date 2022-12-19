// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::api::Factory

#include "api/Factory.hpp"  // IWYU pragma: associated

#include <HDPath.pb.h>
#include <Identifier.pb.h>
#include <boost/endian/buffers.hpp>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <utility>

#include "internal/api/Factory.hpp"
#include "internal/core/Core.hpp"
#include "internal/core/String.hpp"
#include "internal/core/identifier/Factory.hpp"
#include "internal/core/identifier/Identifier.hpp"
#include "internal/otx/common/Cheque.hpp"
#include "internal/otx/common/Item.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Hash.hpp"  // IWYU pragma: keep
#include "opentxs/api/crypto/Util.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/identifier/Algorithm.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Type.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Types.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/crypto/HashType.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto FactoryAPI(const api::Crypto& crypto) noexcept
    -> std::shared_ptr<api::Factory>
{
    using ReturnType = api::imp::Factory;

    return std::make_shared<ReturnType>(crypto);
}
}  // namespace opentxs::factory

namespace opentxs::api::imp
{
template <typename IDType>
auto Factory::id_type() noexcept -> identifier::Type
{
    return identifier::Type::invalid;
}

template <>
auto Factory::id_type<identifier::Generic>() noexcept -> identifier::Type
{
    return identifier::Type::generic;
}

template <>
auto Factory::id_type<identifier::Notary>() noexcept -> identifier::Type
{
    return identifier::Type::notary;
}

template <>
auto Factory::id_type<identifier::Nym>() noexcept -> identifier::Type
{
    return identifier::Type::nym;
}

template <>
auto Factory::id_type<identifier::UnitDefinition>() noexcept -> identifier::Type
{
    return identifier::Type::unitdefinition;
}

template <typename IDType>
auto Factory::id_from_base58(
    const std::string_view base58,
    allocator_type alloc) const noexcept -> IDType
{
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

        if (bytes.size() < identifier_header_) {

            throw std::runtime_error{"input too short (header)"};
        }

        const auto* i = reinterpret_cast<const std::byte*>(bytes.data());
        const auto algo = [&] {
            using Type = identifier::Algorithm;

            switch (std::to_integer<std::uint8_t>(*i)) {
                case static_cast<std::uint8_t>(Type::sha256): {

                    return Type::sha256;
                }
                case static_cast<std::uint8_t>(Type::blake2b160): {

                    return Type::blake2b160;
                }
                case static_cast<std::uint8_t>(Type::blake2b256): {

                    return Type::blake2b256;
                }
                default: {

                    throw std::runtime_error{"unknown algorithm"};
                }
            }
        }();
        std::advance(i, 1_z);
        const auto expectedType = id_type<IDType>();
        const auto type = [&] {
            auto buf = boost::endian::little_uint16_buf_t{};
            static_assert(sizeof(buf) < identifier_header_);
            std::memcpy(
                static_cast<void*>(std::addressof(buf)), i, sizeof(buf));
            using Type = identifier::Type;

            switch (buf.value()) {
                case static_cast<std::uint16_t>(Type::generic): {

                    return Type::generic;
                }
                case static_cast<std::uint16_t>(Type::nym): {

                    return Type::nym;
                }
                case static_cast<std::uint16_t>(Type::notary): {

                    return Type::notary;
                }
                case static_cast<std::uint16_t>(Type::unitdefinition): {

                    return Type::unitdefinition;
                }
                default: {

                    throw std::runtime_error{"unknown algorithm"};
                }
            }
        }();
        const auto effectiveType = [&] {
            constexpr auto generic = identifier::Type::generic;

            if (generic == type) {

                return expectedType;
            } else {
                if (type != expectedType) {
                    log(OT_PRETTY_CLASS())("instantiating ")(print(type))(
                        " identifier as ")(print(expectedType))
                        .Flush();
                }

                return type;
            }
        }();
        const auto hash = bytes.Bytes().substr(identifier_header_);
        const auto goodHash =
            hash.empty() ||
            (hash.size() == identifier_expected_hash_bytes(algo));

        if (false == goodHash) {

            throw std::runtime_error{"wrong number of bytes in hash"};
        }

        return factory::Identifier(effectiveType, algo, hash, std::move(alloc));
    } catch (const std::exception& e) {
        log(OT_PRETTY_CLASS())(e.what()).Flush();

        return factory::IdentifierInvalid(std::move(alloc));
    }
}

template <typename IDType>
auto Factory::id_from_hash(
    const ReadView bytes,
    const identifier::Algorithm type,
    allocator_type alloc) const noexcept -> IDType
{
    auto out = IDType{std::move(alloc)};
    const auto expected = identifier_expected_hash_bytes(type);

    if (const auto size = bytes.size(); size == expected) {
        out.Assign(bytes);
    } else {
        LogError()(OT_PRETTY_CLASS())("expected ")(
            bytes)(" bytes but supplied hash is ")(size)(" bytes")
            .Flush();
    }

    return out;
}

template <typename IDType>
auto Factory::id_from_preimage(
    const identifier::Algorithm type,
    const ReadView preimage,
    allocator_type alloc) const noexcept -> IDType
{
    try {
        const auto hashType = [&] {
            using Type = identifier::Algorithm;
            using Hash = opentxs::crypto::HashType;

            switch (type) {
                case Type::sha256: {

                    return Hash::Sha256;
                }
                case Type::blake2b160: {

                    return Hash::Blake2b160;
                }
                case Type::blake2b256: {

                    return Hash::Blake2b256;
                }
                case Type::invalid:
                default: {

                    throw std::runtime_error("unknown algorithm");
                }
            }
        }();
        auto out = IDType{alloc};
        const auto rc =
            crypto_.Hash().Digest(hashType, preimage, out.WriteInto());

        if (false == rc) {

            throw std::runtime_error("failed to calculate digest");
        }

        return out;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return factory::IdentifierInvalid(alloc);
    }
}

template <typename IDType>
auto Factory::id_from_preimage(
    const identifier::Algorithm type,
    const ProtobufType& proto,
    allocator_type alloc) const noexcept -> IDType
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
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return factory::IdentifierInvalid(alloc);
    }
}

template <typename IDType>
auto Factory::id_from_protobuf(
    const proto::Identifier& proto,
    allocator_type alloc) const noexcept -> IDType
{
    try {
        const auto expectedType = id_type<IDType>();
        const auto type = [&] {
            using Type = identifier::Type;

            switch (proto.type()) {
                case static_cast<std::uint32_t>(Type::generic): {

                    return expectedType;
                }
                case static_cast<std::uint32_t>(Type::nym): {

                    return Type::nym;
                }
                case static_cast<std::uint32_t>(Type::notary): {

                    return Type::notary;
                }
                case static_cast<std::uint32_t>(Type::unitdefinition): {

                    return Type::unitdefinition;
                }
                default: {

                    throw std::runtime_error{"unknown identifier type"};
                }
            }
        }();
        const auto algo = [&] {
            using Type = identifier::Algorithm;

            switch (proto.algorithm()) {
                case static_cast<std::uint32_t>(Type::sha256): {

                    return Type::sha256;
                }
                case static_cast<std::uint32_t>(Type::blake2b160): {

                    return Type::blake2b160;
                }
                case static_cast<std::uint32_t>(Type::blake2b256): {

                    return Type::blake2b256;
                }
                default: {

                    throw std::runtime_error{"unknown algorithm"};
                }
            }
        }();

        constexpr auto generic = identifier::Type::generic;

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

        return factory::Identifier(type, algo, hash, std::move(alloc));
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return factory::IdentifierInvalid(std::move(alloc));
    }
}

template <typename IDType>
auto Factory::id_from_random(
    const identifier::Algorithm type,
    allocator_type alloc) const noexcept -> IDType
{
    try {
        auto out = IDType{std::move(alloc)};
        const auto size = identifier_expected_hash_bytes(type);

        if (0_uz == size) { throw std::runtime_error{"invalid hash type"}; }

        if (false == out.SetSize(size)) {
            throw std::runtime_error{"failed to reserve space for hash"};
        }

        OT_ASSERT(out.size() == size);

        if (false == crypto_.Util().RandomizeMemory(out.data(), out.size())) {
            throw std::runtime_error{"failed to randomize hash"};
        }

        return out;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::api::imp

namespace opentxs::api::imp
{
Factory::Factory(const api::Crypto& crypto) noexcept
    : crypto_(crypto)
{
}

auto Factory::Identifier(
    const identity::wot::claim::ClaimType type,
    const proto::HDPath& path,
    allocator_type alloc) const noexcept -> identifier::Generic
{
    const auto preimage = [&] {
        auto output = [&]() -> ByteArray {
            const auto buf = boost::endian::little_uint32_buf_t{
                static_cast<std::uint32_t>(type)};
            static_assert(sizeof(type) == sizeof(buf));

            return {
                reinterpret_cast<const std::byte*>(std::addressof(buf)),
                sizeof(buf),
                alloc};
        }();
        output.Concatenate(path.root());

        for (const auto& child : path.child()) {
            const auto buf = boost::endian::little_uint32_buf_t{child};
            static_assert(sizeof(child) == sizeof(buf));
            output.Concatenate(std::addressof(buf), sizeof(buf));
        }

        return output;
    }();

    return IdentifierFromPreimage(preimage.Bytes(), std::move(alloc));
}

auto Factory::Identifier(const Cheque& cheque, allocator_type alloc)
    const noexcept -> identifier::Generic
{
    const auto preimage = String::Factory(cheque);

    return IdentifierFromPreimage(preimage->Bytes(), std::move(alloc));
}

auto Factory::Identifier(const Contract& contract, allocator_type alloc)
    const noexcept -> identifier::Generic
{
    const auto preimage = String::Factory(contract);

    return IdentifierFromPreimage(preimage->Bytes(), std::move(alloc));
}

auto Factory::Identifier(const Item& item, allocator_type alloc) const noexcept
    -> identifier::Generic
{
    const auto preimage = String::Factory(item);

    return IdentifierFromPreimage(preimage->Bytes(), std::move(alloc));
}

auto Factory::Identifier(const proto::Identifier& in, allocator_type alloc)
    const noexcept -> identifier::Generic
{
    return id_from_protobuf<identifier::Generic>(in, std::move(alloc));
}

auto Factory::IdentifierFromBase58(
    const std::string_view base58,
    allocator_type alloc) const noexcept -> identifier::Generic
{
    return id_from_base58<identifier::Generic>(base58, std::move(alloc));
}

auto Factory::IdentifierFromHash(const ReadView bytes, allocator_type alloc)
    const noexcept -> identifier::Generic
{
    return IdentifierFromHash(
        bytes, default_identifier_algorithm(), std::move(alloc));
}

auto Factory::IdentifierFromHash(
    const ReadView bytes,
    const identifier::Algorithm type,
    allocator_type alloc) const noexcept -> identifier::Generic
{
    return id_from_hash<identifier::Generic>(bytes, type, std::move(alloc));
}

auto Factory::IdentifierFromPreimage(
    const ReadView preimage,
    allocator_type alloc) const noexcept -> identifier::Generic
{
    return IdentifierFromPreimage(
        preimage, default_identifier_algorithm(), std::move(alloc));
}

auto Factory::IdentifierFromPreimage(
    const ReadView preimage,
    const identifier::Algorithm type,
    allocator_type alloc) const noexcept -> identifier::Generic
{
    return id_from_preimage<identifier::Generic>(
        type, preimage, std::move(alloc));
}

auto Factory::IdentifierFromPreimage(
    const ProtobufType& proto,
    allocator_type alloc) const noexcept -> identifier::Generic
{
    return IdentifierFromPreimage(
        proto, default_identifier_algorithm(), std::move(alloc));
}

auto Factory::IdentifierFromPreimage(
    const ProtobufType& proto,
    const identifier::Algorithm type,
    allocator_type alloc) const noexcept -> identifier::Generic
{
    return id_from_preimage<identifier::Generic>(type, proto, std::move(alloc));
}

auto Factory::IdentifierFromRandom(allocator_type alloc) const noexcept
    -> identifier::Generic
{
    return IdentifierFromRandom(
        default_identifier_algorithm(), std::move(alloc));
}

auto Factory::IdentifierFromRandom(
    const identifier::Algorithm type,
    allocator_type alloc) const noexcept -> identifier::Generic
{
    return id_from_random<identifier::Generic>(type, std::move(alloc));
}

auto Factory::NotaryID(const proto::Identifier& in, allocator_type alloc)
    const noexcept -> identifier::Notary
{
    return id_from_protobuf<identifier::Notary>(in, std::move(alloc));
}

auto Factory::NotaryIDConvertSafe(
    const identifier::Generic& in,
    allocator_type alloc) const noexcept -> identifier::Notary
{
    switch (in.Type()) {
        case identifier::Type::notary:
        case identifier::Type::generic: {

            return factory::Identifier(
                identifier::Type::notary,
                in.Algorithm(),
                in.Bytes(),
                std::move(alloc));
        }
        case identifier::Type::invalid:
        case identifier::Type::nym:
        case identifier::Type::unitdefinition:
        default: {

            return factory::IdentifierInvalid(std::move(alloc));
        }
    }
}

auto Factory::NotaryIDFromBase58(
    const std::string_view base58,
    allocator_type alloc) const noexcept -> identifier::Notary
{
    return id_from_base58<identifier::Notary>(base58, std::move(alloc));
}

auto Factory::NotaryIDFromHash(const ReadView bytes, allocator_type alloc)
    const noexcept -> identifier::Notary
{
    return NotaryIDFromHash(
        bytes, default_identifier_algorithm(), std::move(alloc));
}

auto Factory::NotaryIDFromHash(
    const ReadView bytes,
    const identifier::Algorithm type,
    allocator_type alloc) const noexcept -> identifier::Notary
{
    return id_from_hash<identifier::Notary>(bytes, type, std::move(alloc));
}

auto Factory::NotaryIDFromPreimage(
    const ReadView preimage,
    allocator_type alloc) const noexcept -> identifier::Notary
{
    return NotaryIDFromPreimage(
        preimage, default_identifier_algorithm(), std::move(alloc));
}

auto Factory::NotaryIDFromPreimage(
    const ReadView preimage,
    const identifier::Algorithm type,
    allocator_type alloc) const noexcept -> identifier::Notary
{
    return id_from_preimage<identifier::Notary>(
        type, preimage, std::move(alloc));
}

auto Factory::NotaryIDFromPreimage(
    const ProtobufType& proto,
    allocator_type alloc) const noexcept -> identifier::Notary
{
    return NotaryIDFromPreimage(
        proto, default_identifier_algorithm(), std::move(alloc));
}

auto Factory::NotaryIDFromPreimage(
    const ProtobufType& proto,
    const identifier::Algorithm type,
    allocator_type alloc) const noexcept -> identifier::Notary
{
    return id_from_preimage<identifier::Notary>(type, proto, std::move(alloc));
}

auto Factory::NotaryIDFromRandom(allocator_type alloc) const noexcept
    -> identifier::Notary
{
    return NotaryIDFromRandom(default_identifier_algorithm(), std::move(alloc));
}

auto Factory::NotaryIDFromRandom(
    const identifier::Algorithm type,
    allocator_type alloc) const noexcept -> identifier::Notary
{
    return id_from_random<identifier::Notary>(type, std::move(alloc));
}

auto Factory::NymID(const proto::Identifier& in, allocator_type alloc)
    const noexcept -> identifier::Nym
{
    return id_from_protobuf<identifier::Nym>(in, std::move(alloc));
}

auto Factory::NymIDConvertSafe(
    const identifier::Generic& in,
    allocator_type alloc) const noexcept -> identifier::Nym
{
    switch (in.Type()) {
        case identifier::Type::nym:
        case identifier::Type::generic: {

            return factory::Identifier(
                identifier::Type::nym,
                in.Algorithm(),
                in.Bytes(),
                std::move(alloc));
        }
        case identifier::Type::invalid:
        case identifier::Type::unitdefinition:
        case identifier::Type::notary:
        default: {

            return factory::IdentifierInvalid(std::move(alloc));
        }
    }
}

auto Factory::NymIDFromBase58(
    const std::string_view base58,
    allocator_type alloc) const noexcept -> identifier::Nym
{
    return id_from_base58<identifier::Nym>(base58, std::move(alloc));
}

auto Factory::NymIDFromHash(const ReadView bytes, allocator_type alloc)
    const noexcept -> identifier::Nym
{
    return NymIDFromHash(
        bytes, default_identifier_algorithm(), std::move(alloc));
}

auto Factory::NymIDFromHash(
    const ReadView bytes,
    const identifier::Algorithm type,
    allocator_type alloc) const noexcept -> identifier::Nym
{
    return id_from_hash<identifier::Nym>(bytes, type, std::move(alloc));
}

auto Factory::NymIDFromPreimage(const ReadView preimage, allocator_type alloc)
    const noexcept -> identifier::Nym
{
    return NymIDFromPreimage(
        preimage, default_identifier_algorithm(), std::move(alloc));
}

auto Factory::NymIDFromPreimage(
    const ReadView preimage,
    const identifier::Algorithm type,
    allocator_type alloc) const noexcept -> identifier::Nym
{
    return id_from_preimage<identifier::Nym>(type, preimage, std::move(alloc));
}

auto Factory::NymIDFromRandom(allocator_type alloc) const noexcept
    -> identifier::Nym
{
    return NymIDFromRandom(default_identifier_algorithm(), std::move(alloc));
}

auto Factory::NymIDFromRandom(
    const identifier::Algorithm type,
    allocator_type alloc) const noexcept -> identifier::Nym
{
    return id_from_random<identifier::Nym>(type, std::move(alloc));
}

auto Factory::Secret(const std::size_t bytes) const noexcept -> opentxs::Secret
{
    return factory::Secret(bytes);
}

auto Factory::SecretFromBytes(const ReadView bytes) const noexcept
    -> opentxs::Secret
{
    return factory::Secret(bytes, true);
}

auto Factory::SecretFromText(const std::string_view text) const noexcept
    -> opentxs::Secret
{
    return factory::Secret(text, false);
}

auto Factory::UnitID(const proto::Identifier& in, allocator_type alloc)
    const noexcept -> identifier::UnitDefinition
{
    return id_from_protobuf<identifier::UnitDefinition>(in, std::move(alloc));
}

auto Factory::UnitIDConvertSafe(
    const identifier::Generic& in,
    allocator_type alloc) const noexcept -> identifier::UnitDefinition
{
    switch (in.Type()) {
        case identifier::Type::unitdefinition:
        case identifier::Type::generic: {

            return factory::Identifier(
                identifier::Type::unitdefinition,
                in.Algorithm(),
                in.Bytes(),
                std::move(alloc));
        }
        case identifier::Type::invalid:
        case identifier::Type::nym:
        case identifier::Type::notary:
        default: {

            return factory::IdentifierInvalid(std::move(alloc));
        }
    }
}

auto Factory::UnitIDFromBase58(
    const std::string_view base58,
    allocator_type alloc) const noexcept -> identifier::UnitDefinition
{
    return id_from_base58<identifier::UnitDefinition>(base58, std::move(alloc));
}

auto Factory::UnitIDFromHash(const ReadView bytes, allocator_type alloc)
    const noexcept -> identifier::UnitDefinition
{
    return UnitIDFromHash(
        bytes, default_identifier_algorithm(), std::move(alloc));
}

auto Factory::UnitIDFromHash(
    const ReadView bytes,
    const identifier::Algorithm type,
    allocator_type alloc) const noexcept -> identifier::UnitDefinition
{
    return id_from_hash<identifier::UnitDefinition>(
        bytes, type, std::move(alloc));
}

auto Factory::UnitIDFromPreimage(const ReadView preimage, allocator_type alloc)
    const noexcept -> identifier::UnitDefinition
{
    return UnitIDFromPreimage(
        preimage, default_identifier_algorithm(), std::move(alloc));
}

auto Factory::UnitIDFromPreimage(
    const ReadView preimage,
    const identifier::Algorithm type,
    allocator_type alloc) const noexcept -> identifier::UnitDefinition
{
    return id_from_preimage<identifier::UnitDefinition>(
        type, preimage, std::move(alloc));
}

auto Factory::UnitIDFromPreimage(
    const ProtobufType& proto,
    allocator_type alloc) const noexcept -> identifier::UnitDefinition
{
    return UnitIDFromPreimage(
        proto, default_identifier_algorithm(), std::move(alloc));
}

auto Factory::UnitIDFromPreimage(
    const ProtobufType& proto,
    const identifier::Algorithm type,
    allocator_type alloc) const noexcept -> identifier::UnitDefinition
{
    return id_from_preimage<identifier::UnitDefinition>(
        type, proto, std::move(alloc));
}

auto Factory::UnitIDFromRandom(allocator_type alloc) const noexcept
    -> identifier::UnitDefinition
{
    return UnitIDFromRandom(default_identifier_algorithm(), std::move(alloc));
}

auto Factory::UnitIDFromRandom(
    const identifier::Algorithm type,
    allocator_type alloc) const noexcept -> identifier::UnitDefinition
{
    return id_from_random<identifier::UnitDefinition>(type, std::move(alloc));
}
}  // namespace opentxs::api::imp
