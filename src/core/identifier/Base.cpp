// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"              // IWYU pragma: associated
#include "1_Internal.hpp"            // IWYU pragma: associated
#include "core/identifier/Base.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <robin_hood.h>
#include <cstdint>
#include <cstring>
#include <iosfwd>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include "Proto.hpp"
#include "core/Data.hpp"
#include "internal/core/identifier/Factory.hpp"
#include "internal/protobuf/Check.hpp"
#include "internal/protobuf/verify/Identifier.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Item.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/contract/ContractType.hpp"
#include "opentxs/core/identifier/Algorithm.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/Type.hpp"
#include "opentxs/core/identifier/Types.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "serialization/protobuf/HDPath.pb.h"
#include "serialization/protobuf/Identifier.pb.h"

template class opentxs::Pimpl<opentxs::Identifier>;

namespace std
{
auto less<opentxs::Pimpl<opentxs::Identifier>>::operator()(
    const opentxs::OTIdentifier& lhs,
    const opentxs::OTIdentifier& rhs) const -> bool
{
    return lhs.get() < rhs.get();
}
}  // namespace std

namespace opentxs::factory
{
using ReturnType = implementation::Identifier;
auto decode_proto(const proto::Identifier& in) noexcept
    -> std::unique_ptr<ReturnType>;

auto IdentifierGeneric() noexcept -> std::unique_ptr<opentxs::Identifier>
{
    return std::make_unique<ReturnType>();
}

auto IdentifierGeneric(const proto::Identifier& in) noexcept
    -> std::unique_ptr<opentxs::Identifier>
{
    return decode_proto(in);
}

auto IdentifierNym() noexcept -> std::unique_ptr<opentxs::identifier::Nym>
{
    return std::make_unique<ReturnType>();
}

auto IdentifierNym(const proto::Identifier& in) noexcept
    -> std::unique_ptr<opentxs::identifier::Nym>
{
    return decode_proto(in);
}

auto IdentifierNotary() noexcept -> std::unique_ptr<opentxs::identifier::Server>
{
    return std::make_unique<ReturnType>();
}

auto IdentifierNotary(const proto::Identifier& in) noexcept
    -> std::unique_ptr<opentxs::identifier::Server>
{
    return decode_proto(in);
}

auto IdentifierUnit() noexcept
    -> std::unique_ptr<opentxs::identifier::UnitDefinition>
{
    return std::make_unique<ReturnType>();
}

auto IdentifierUnit(const proto::Identifier& in) noexcept
    -> std::unique_ptr<opentxs::identifier::UnitDefinition>
{
    return decode_proto(in);
}

auto decode_proto(const proto::Identifier& in) noexcept
    -> std::unique_ptr<ReturnType>
{
    try {
        if (false == proto::Validate(in, VERBOSE)) {
            throw std::runtime_error{"invalid serialized identifier"};
        }

        const auto algorithm =
            static_cast<identifier::Algorithm>(in.algorithm());
        const auto type = static_cast<identifier::Type>(in.type());

        switch (algorithm) {
            case identifier::Algorithm::sha256:
            case identifier::Algorithm::blake2b160:
            case identifier::Algorithm::blake2b256: {
            } break;
            case identifier::Algorithm::invalid:
            default: {
                throw std::runtime_error{
                    "invalid or unknown identifier algorithm"};
            }
        }

        switch (type) {
            case identifier::Type::generic:
            case identifier::Type::nym:
            case identifier::Type::notary:
            case identifier::Type::unitdefinition: {
            } break;
            case identifier::Type::invalid:
            default: {
                throw std::runtime_error{"invalid or unknown identifier type"};
            }
        }

        const auto& hash = in.hash();
        const auto* start = reinterpret_cast<const std::uint8_t*>(hash.data());
        const auto* end = std::next(start, hash.size());

        return std::make_unique<ReturnType>(
            ReturnType::Vector{start, end}, algorithm, type);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();
        static const auto blank = ReturnType::Vector{};

        return std::make_unique<ReturnType>(
            blank, identifier::Algorithm::invalid, identifier::Type::invalid);
    }
}
}  // namespace opentxs::factory

namespace opentxs
{
using Imp = implementation::Identifier;

auto default_identifier_algorithm() noexcept -> identifier::Algorithm
{
    return identifier::Algorithm::blake2b256;
}

auto operator==(const OTIdentifier& lhs, const Identifier& rhs) noexcept -> bool
{
    return lhs.get().operator==(rhs);
}

auto operator==(const OTNymID& lhs, const opentxs::Identifier& rhs) noexcept
    -> bool
{
    return lhs.get().operator==(rhs);
}

auto operator==(const OTServerID& lhs, const opentxs::Identifier& rhs) noexcept
    -> bool
{
    return lhs.get().operator==(rhs);
}

auto operator==(const OTUnitID& lhs, const opentxs::Identifier& rhs) noexcept
    -> bool
{
    return lhs.get().operator==(rhs);
}

auto operator!=(const OTIdentifier& lhs, const Identifier& rhs) noexcept -> bool
{
    return lhs.get().operator!=(rhs);
}

auto operator!=(const OTNymID& lhs, const opentxs::Identifier& rhs) noexcept
    -> bool
{
    return lhs.get().operator!=(rhs);
}

auto operator!=(const OTServerID& lhs, const opentxs::Identifier& rhs) noexcept
    -> bool
{
    return lhs.get().operator!=(rhs);
}

auto operator!=(const OTUnitID& lhs, const opentxs::Identifier& rhs) noexcept
    -> bool
{
    return lhs.get().operator!=(rhs);
}

auto operator<(const OTIdentifier& lhs, const opentxs::Identifier& rhs) noexcept
    -> bool
{
    return lhs.get().operator<(rhs);
}

auto operator<(const OTNymID& lhs, const opentxs::Identifier& rhs) noexcept
    -> bool
{
    return lhs.get().operator<(rhs);
}

auto operator<(const OTServerID& lhs, const opentxs::Identifier& rhs) noexcept
    -> bool
{
    return lhs.get().operator<(rhs);
}

auto operator<(const OTUnitID& lhs, const opentxs::Identifier& rhs) noexcept
    -> bool
{
    return lhs.get().operator<(rhs);
}

auto operator>(const OTIdentifier& lhs, const opentxs::Identifier& rhs) noexcept
    -> bool
{
    return lhs.get().operator>(rhs);
}

auto operator>(const OTNymID& lhs, const opentxs::Identifier& rhs) noexcept
    -> bool
{
    return lhs.get().operator>(rhs);
}

auto operator>(const OTServerID& lhs, const opentxs::Identifier& rhs) noexcept
    -> bool
{
    return lhs.get().operator>(rhs);
}

auto operator>(const OTUnitID& lhs, const opentxs::Identifier& rhs) noexcept
    -> bool
{
    return lhs.get().operator>(rhs);
}

auto operator<=(
    const OTIdentifier& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool
{
    return lhs.get().operator<=(rhs);
}

auto operator<=(const OTNymID& lhs, const opentxs::Identifier& rhs) noexcept
    -> bool
{
    return lhs.get().operator<=(rhs);
}

auto operator<=(const OTServerID& lhs, const opentxs::Identifier& rhs) noexcept
    -> bool
{
    return lhs.get().operator<=(rhs);
}

auto operator<=(const OTUnitID& lhs, const opentxs::Identifier& rhs) noexcept
    -> bool
{
    return lhs.get().operator<=(rhs);
}

auto operator>=(
    const OTIdentifier& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool
{
    return lhs.get().operator>=(rhs);
}

auto operator>=(const OTNymID& lhs, const opentxs::Identifier& rhs) noexcept
    -> bool
{
    return lhs.get().operator>=(rhs);
}

auto operator>=(const OTServerID& lhs, const opentxs::Identifier& rhs) noexcept
    -> bool
{
    return lhs.get().operator>=(rhs);
}

auto operator>=(const OTUnitID& lhs, const opentxs::Identifier& rhs) noexcept
    -> bool
{
    return lhs.get().operator>=(rhs);
}

auto print(identifier::Algorithm in) noexcept -> const char*
{
    static const auto map =
        robin_hood::unordered_flat_map<identifier::Algorithm, const char*>{
            {identifier::Algorithm::invalid, "invalid"},
            {identifier::Algorithm::sha256, "sha256"},
            {identifier::Algorithm::blake2b160, "blake2b160"},
            {identifier::Algorithm::blake2b256, "blake2b256"},
        };

    try {

        return map.at(in);
    } catch (...) {

        return "unknown";
    }
}

auto print(identifier::Type in) noexcept -> const char*
{
    static const auto map =
        robin_hood::unordered_flat_map<identifier::Type, const char*>{
            {identifier::Type::invalid, "invalid"},
            {identifier::Type::generic, "generic"},
            {identifier::Type::nym, "nym"},
            {identifier::Type::notary, "notary"},
            {identifier::Type::unitdefinition, "unit definition"},
        };

    try {

        return map.at(in);
    } catch (...) {

        return "unknown";
    }
}

auto translate(identifier::Type in) noexcept -> contract::Type
{
    static const auto map =
        robin_hood::unordered_flat_map<identifier::Type, contract::Type>{
            {identifier::Type::invalid, contract::Type::invalid},
            {identifier::Type::generic, contract::Type::invalid},
            {identifier::Type::nym, contract::Type::nym},
            {identifier::Type::notary, contract::Type::notary},
            {identifier::Type::unitdefinition, contract::Type::unit},
        };

    try {

        return map.at(in);
    } catch (...) {

        return contract::Type::invalid;
    }
}

auto Identifier::Factory() -> OTIdentifier
{
    auto out = std::make_unique<Imp>(identifier::Type::generic);

    return OTIdentifier(out.release());
}

auto identifier::Nym::Factory() -> OTNymID
{
    auto out = std::make_unique<Imp>(identifier::Type::nym);

    return OTNymID(out.release());
}

auto identifier::Server::Factory() -> OTServerID
{
    auto out = std::make_unique<Imp>(identifier::Type::notary);

    return OTServerID(out.release());
}

auto identifier::UnitDefinition::Factory() -> OTUnitID
{
    auto out = std::make_unique<Imp>(identifier::Type::unitdefinition);

    return OTUnitID(out.release());
}

auto Identifier::Factory(const Identifier& rhs) -> OTIdentifier
{
    return OTIdentifier(
#ifndef _WIN32
        rhs.clone()
#else
        dynamic_cast<Identifier*>(rhs.clone())
#endif
    );
}

auto Identifier::Factory(const std::string& rhs) -> OTIdentifier
{
    auto out = std::make_unique<Imp>(Imp::decode(rhs));

    return OTIdentifier(out.release());
}

auto identifier::Nym::Factory(const std::string& rhs) -> OTNymID
{
    auto out = std::make_unique<Imp>(identifier::Type::nym);
    out->SetString(rhs);

    return OTNymID(out.release());
}

auto identifier::Server::Factory(const std::string& rhs) -> OTServerID
{
    auto out = std::make_unique<Imp>(identifier::Type::notary);
    out->SetString(rhs);

    return OTServerID(out.release());
}

auto identifier::UnitDefinition::Factory(const std::string& rhs) -> OTUnitID
{
    auto out = std::make_unique<Imp>(identifier::Type::unitdefinition);
    out->SetString(rhs);

    return OTUnitID(out.release());
}

auto Identifier::Factory(const String& rhs) -> OTIdentifier
{
    auto out = std::make_unique<Imp>(Imp::decode(rhs.Bytes()));

    return OTIdentifier(out.release());
}

auto identifier::Nym::Factory(const String& rhs) -> OTNymID
{
    auto out = std::make_unique<Imp>(identifier::Type::nym);
    out->SetString(rhs);

    return OTNymID(out.release());
}

auto identifier::Server::Factory(const String& rhs) -> OTServerID
{
    auto out = std::make_unique<Imp>(identifier::Type::notary);
    out->SetString(rhs);

    return OTServerID(out.release());
}

auto identifier::UnitDefinition::Factory(const String& rhs) -> OTUnitID
{
    auto out = std::make_unique<Imp>(identifier::Type::unitdefinition);
    out->SetString(rhs);

    return OTUnitID(out.release());
}

auto Identifier::Factory(const identity::Nym& nym) -> OTIdentifier
{
    auto out = std::make_unique<Imp>(nym);

    return OTIdentifier(out.release());
}

auto identifier::Nym::Factory(const identity::Nym& rhs) -> OTNymID
{
    auto out = std::make_unique<Imp>(rhs);

    return OTNymID(out.release());
}

auto Identifier::Factory(const Contract& contract) -> OTIdentifier
{
    auto out = std::make_unique<Imp>(contract);

    return OTIdentifier(out.release());
}

auto Identifier::Factory(const Cheque& cheque) -> OTIdentifier
{
    return OTIdentifier(
        implementation::Identifier::contract_contents_to_identifier(cheque));
}

auto Identifier::Factory(const Item& item) -> OTIdentifier
{
    return OTIdentifier(
        implementation::Identifier::contract_contents_to_identifier(item));
}

auto Identifier::Factory(
    const contact::ClaimType type,
    const proto::HDPath& path) -> OTIdentifier
{
    auto out = std::make_unique<Imp>(type, path);

    return OTIdentifier(out.release());
}

auto Identifier::Random() -> OTIdentifier
{
    auto output = Identifier::Factory();
    output->Randomize();

    return output;
}

auto Identifier::Validate(const std::string& input) -> bool
{
    if (input.empty()) { return false; }

    const auto id = Factory(input);

    return (0 < id->size());
}
}  // namespace opentxs

namespace opentxs::implementation
{
Identifier::Identifier(
    Vector&& data,
    identifier::Algorithm algorithm,
    identifier::Type type) noexcept
    : ot_super(std::move(data))
    , algorithm_(algorithm)
    , type_(type)
{
}

Identifier::Identifier(Decoded&& data) noexcept
    : Identifier(
          std::move(std::get<0>(data)),
          std::get<1>(data),
          std::get<2>(data))
{
}

Identifier::Identifier(
    const Vector& data,
    const identifier::Algorithm algorithm,
    const identifier::Type type) noexcept
    : Identifier(Vector{data}, algorithm, type)
{
}

Identifier::Identifier(identifier::Type type) noexcept
    : Identifier({}, default_identifier_algorithm(), type)
{
}

Identifier::Identifier() noexcept
    : Identifier({}, default_identifier_algorithm(), identifier::Type::generic)
{
}

Identifier::Identifier(const Contract& theContract) noexcept
    : Identifier()
{
    (const_cast<Contract&>(theContract)).GetIdentifier(*this);
}

Identifier::Identifier(const identity::Nym& theNym) noexcept
    : Identifier({}, default_identifier_algorithm(), identifier::Type::nym)
{
    (const_cast<identity::Nym&>(theNym)).GetIdentifier(*this);
}

Identifier::Identifier(const Identifier& rhs) noexcept
    : Identifier(rhs.data_, rhs.algorithm_, rhs.type_)
{
}

Identifier::Identifier(
    const contact::ClaimType type,
    const proto::HDPath& path) noexcept
    : Identifier()
{
    CalculateDigest(path_to_data(type, path)->Bytes(), algorithm_);
}

auto Identifier::Assign(const void* data, const std::size_t size) noexcept
    -> bool
{
    if (const auto bytes = hash_bytes(algorithm_); bytes < size) {
        LogError()(OT_PRETTY_CLASS())("Too many size specified (")(
            size)(") vs max (")(bytes)(")")
            .Flush();

        return false;
    }

    return Data::Assign(data, size);
}

auto Identifier::CalculateDigest(
    const ReadView bytes,
    identifier::Algorithm type) -> bool
{
    algorithm_ = type;

    return Context().Crypto().Hash().Digest(
        IDToHashType(algorithm_), bytes, WriteInto());
}

auto Identifier::clone() const -> Identifier*
{
    return std::make_unique<Identifier>(*this).release();
}

auto Identifier::Concatenate(const void* data, const std::size_t size) noexcept
    -> bool
{
    const auto max = [&]() -> std::size_t {
        const auto target = hash_bytes(algorithm_);

        if (const auto used = this->size(); used >= target) {

            return 0;
        } else {

            return target - used;
        }
    }();

    if (size > max) {
        LogError()(OT_PRETTY_CLASS())("Too many bytes specified (")(
            size)(") vs max (")(max)(")")
            .Flush();

        return false;
    }

    return Data::Concatenate(data, size);
}

auto Identifier::contract_contents_to_identifier(const Contract& in)
    -> Identifier*
{
    auto output = std::make_unique<Identifier>();

    OT_ASSERT(output);

    const auto preimage = String::Factory(in);
    output->CalculateDigest(preimage->Bytes(), default_identifier_algorithm());

    return output.release();
}

auto Identifier::decode(ReadView in) noexcept -> Decoded
{
    auto output =
        Decoded{{}, identifier::Algorithm::invalid, identifier::Type::invalid};
    auto& [data, algorithm, type] = output;

    if (0u == in.size()) { return output; }

    try {
        if (minimum_encoded_bytes_ > in.size()) {
            throw std::runtime_error{"input too short"};
        }

        const auto bytes = [&] {
            constexpr auto prefix = std::size_t{2u};
            auto* i = in.data();

            if (0u != std::memcmp(i, prefix_, prefix)) {
                throw std::runtime_error{"invalid prefix"};
            }

            std::advance(i, prefix);

            return Context().Crypto().Encode().IdentifierDecode(
                std::string{i, in.size() - prefix});
        }();
        const auto length = bytes.size();

        if (header_bytes_ > length) {
            throw std::runtime_error{"decode error"};
        }

        algorithm = static_cast<identifier::Algorithm>(bytes[0]);

        if (false == is_supported(algorithm)) {
            algorithm = identifier::Algorithm::invalid;

            throw std::runtime_error{"unsupported algorithm"};
        }

        auto* i = reinterpret_cast<const std::uint8_t*>(bytes.data());
        std::advance(i, sizeof(algorithm_));
        const auto serializedType = [&] {
            auto out = boost::endian::little_uint16_buf_t{};
            std::memcpy(static_cast<void*>(&out), i, sizeof(out));
            std::advance(i, sizeof(out));

            return out;
        }();

        type = static_cast<identifier::Type>(serializedType.value());

        if (false == is_supported(type)) {
            algorithm = identifier::Algorithm::invalid;

            throw std::runtime_error{"unsupported type"};
        }

        if (required_payload(algorithm) != length) {
            const auto error = std::string{"invalid payload ("} +
                               std::to_string(length) + " bytes )";

            throw std::runtime_error{error};
        }

        data = Vector{i, i + (length - header_bytes_)};
    } catch (const std::runtime_error& e) {
        LogError()(OT_PRETTY_STATIC(Identifier))(e.what()).Flush();  // FIXME
    }

    return output;
}

// This Identifier is stored in binary form.
// But what if you want a pretty string version of it?
// Just call this function.
auto Identifier::GetString(String& id) const -> void
{
    const auto value = to_string();

    if (0 == value.size()) {
        id.Release();
    } else {
        id.Set(value.c_str());
    }
}

auto Identifier::IDToHashType(const identifier::Algorithm type)
    -> crypto::HashType
{
    static const auto map =
        robin_hood::unordered_flat_map<identifier::Algorithm, crypto::HashType>{
            {identifier::Algorithm::sha256, crypto::HashType::Sha256},
            {identifier::Algorithm::blake2b160, crypto::HashType::Blake2b160},
            {identifier::Algorithm::blake2b256, crypto::HashType::Blake2b256},
        };

    try {

        return map.at(type);
    } catch (...) {

        return crypto::HashType::None;
    }
}

auto Identifier::is_compatible(
    identifier::Type lhs,
    identifier::Type rhs) noexcept -> bool
{
    if (identifier::Type::generic == lhs) { return true; }

    if (identifier::Type::generic == rhs) { return true; }

    return (lhs == rhs);
}

auto Identifier::is_supported(const identifier::Algorithm type) noexcept -> bool
{
    static const auto set =
        robin_hood::unordered_flat_set<identifier::Algorithm>{
            identifier::Algorithm::sha256,
            identifier::Algorithm::blake2b160,
            identifier::Algorithm::blake2b256,
        };

    return 0u < set.count(type);
}

auto Identifier::is_supported(const identifier::Type type) noexcept -> bool
{
    static const auto set = robin_hood::unordered_flat_set<identifier::Type>{
        identifier::Type::generic,
        identifier::Type::nym,
        identifier::Type::notary,
        identifier::Type::unitdefinition,
    };

    return 0u < set.count(type);
}

auto Identifier::path_to_data(
    const contact::ClaimType type,
    const proto::HDPath& path) -> OTData
{
    auto output = Data::Factory(static_cast<const void*>(&type), sizeof(type));
    output += Data::Factory(path.root().c_str(), path.root().size());

    for (const auto& child : path.child()) {
        output += Data::Factory(&child, sizeof(child));
    }

    return output;
}

auto Identifier::hash_bytes(const identifier::Algorithm type) noexcept
    -> std::size_t
{
    static const auto map =
        robin_hood::unordered_flat_map<identifier::Algorithm, std::size_t>{
            {identifier::Algorithm::sha256, 32u},
            {identifier::Algorithm::blake2b160, 20u},
            {identifier::Algorithm::blake2b256, 32u},
        };

    try {

        return map.at(type);
    } catch (...) {

        return 0u;
    }
}

auto Identifier::Randomize(const std::size_t size) -> bool
{
    if (const auto bytes = hash_bytes(algorithm_); bytes != size) {
        LogError()(OT_PRETTY_CLASS())("Incorrect size specified (")(
            size)(") vs required (")(bytes)(")")
            .Flush();

        return false;
    }

    return Data::Randomize(size);
}

auto Identifier::Randomize() -> bool
{
    return Randomize(hash_bytes(algorithm_));
}

auto Identifier::Serialize(proto::Identifier& out) const noexcept -> bool
{
    out.set_version(proto_version_);
    static constexpr auto badAlgo = identifier::Algorithm::invalid;
    static constexpr auto badType = identifier::Type::invalid;

    if ((badAlgo == algorithm_) || (badType == type_)) {
        out.clear_hash();
        out.set_algorithm(static_cast<std::uint32_t>(badAlgo));
        out.set_type(static_cast<std::uint32_t>(badType));

        return true;
    }

    out.set_hash(std::string{Bytes()});
    out.set_algorithm(static_cast<std::uint32_t>(algorithm_));
    out.set_type(static_cast<std::uint32_t>(type_));

    return true;
}

auto Identifier::SetString(const ReadView encoded) -> void
{
    auto [data, algorithm, type] = decode(encoded);

    if (identifier::Algorithm::invalid == algorithm) {
        data_.clear();

        return;
    }

    if (is_compatible(type_, type)) {
        data_.swap(data);
        algorithm_ = algorithm;

        // NOTE upgrading from generic to a more specific type is allowed. The
        // reverse is not
        if (identifier::Type::generic == type_) { type_ = type; }
    } else {
        std::cerr << OT_PRETTY_CLASS() << "trying to import a "
                  << opentxs::print(type) << " string into a "
                  << opentxs::print(type_) << " identifier" << std::endl;
        data_.clear();
        algorithm_ = identifier::Algorithm::invalid;
        type_ = identifier::Type::invalid;
    }
}

auto Identifier::SetString(const String& encoded) -> void
{
    return SetString(encoded.Bytes());
}

auto Identifier::SetString(const std::string& encoded) -> void
{
    return SetString(ReadView{encoded});
}

auto Identifier::str() const -> std::string { return to_string(); }

auto Identifier::swap(opentxs::Identifier& rhs) -> void
{
    auto& input = dynamic_cast<Identifier&>(rhs);
    ot_super::swap(std::move(input));
    std::swap(algorithm_, input.algorithm_);
    std::swap(type_, input.type_);
}

auto Identifier::to_string() const noexcept -> std::string
{
    if (const auto bytes = hash_bytes(algorithm_), len = size(); len != bytes) {
        if (0u != len) {
            LogError()(OT_PRETTY_CLASS())("Incorrect hash size (")(size())(
                ") vs required (")(bytes)(")")
                .Flush();
        }

        return {};
    }

    const auto preimage = [&] {
        auto out = Data::Factory();
        const auto payload = size();

        if (0 == payload) { return out; }

        const auto type = boost::endian::little_uint16_buf_t{
            static_cast<std::uint16_t>(type_)};
        out->resize(sizeof(algorithm_) + sizeof(type) + payload);

        OT_ASSERT(out->size() == required_payload(algorithm_));

        auto* i = static_cast<std::byte*>(out->data());
        std::memcpy(i, &algorithm_, sizeof(algorithm_));
        std::advance(i, sizeof(algorithm_));
        std::memcpy(i, static_cast<const void*>(&type), sizeof(type));
        std::advance(i, sizeof(type));
        std::memcpy(i, data(), payload);
        std::advance(i, payload);

        return out;
    }();
    auto ss = std::stringstream{};

    if (0 < preimage->size()) {
        ss << prefix_;
        ss << Context().Crypto().Encode().IdentifierEncode(preimage);
    }

    return ss.str();
}
}  // namespace opentxs::implementation
