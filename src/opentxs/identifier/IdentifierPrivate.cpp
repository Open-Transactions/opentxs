// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/identifier/IdentifierPrivate.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>
#include <opentxs/protobuf/Identifier.pb.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "internal/core/identifier/Identifier.hpp"
#include "internal/network/zeromq/message/Message.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/AccountType.hpp"  // IWYU pragma: keep
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/crypto/HashType.hpp"            // IWYU pragma: keep
#include "opentxs/identifier/AccountSubtype.hpp"  // IWYU pragma: keep
#include "opentxs/identifier/Algorithm.hpp"       // IWYU pragma: keep
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Type.hpp"  // IWYU pragma: keep
#include "opentxs/identifier/Types.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::identifier
{
static constexpr auto get = [](const auto val) {
    return std::make_pair(static_cast<std::uint32_t>(val), val);
};

auto deserialize_account_subtype(std::uint16_t in) noexcept -> AccountSubtype
{
    using enum AccountSubtype;
    const auto map = frozen::make_unordered_map<std::uint16_t, AccountSubtype>({
        get(invalid_subtype),
        get(custodial_account),
        get(blockchain_account),
        get(blockchain_subaccount),
        get(blockchain_subchain),
    });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return invalid_subtype;
    }
}

auto deserialize_algorithm(std::uint8_t in) noexcept -> Algorithm
{
    using enum Algorithm;
    const auto map = frozen::make_unordered_map<std::uint8_t, Algorithm>({
        get(invalid),
        get(sha256),
        get(blake2b160),
        get(blake2b256),
    });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return invalid;
    }
}

auto deserialize_identifier_type(std::uint16_t in) noexcept -> Type
{
    using enum Type;
    const auto map = frozen::make_unordered_map<std::uint16_t, Type>({
        get(invalid),
        get(generic),
        get(nym),
        get(notary),
        get(unitdefinition),
        get(account),
        get(hdseed),
    });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return invalid;
    }
}

auto get_hash_type(Algorithm in) noexcept(false) -> crypto::HashType
{
    using enum Algorithm;
    using enum crypto::HashType;
    const auto map = frozen::make_unordered_map<Algorithm, crypto::HashType>({
        {sha256, Sha256},
        {blake2b160, Blake2b160},
        {blake2b256, Blake2b256},
    });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        throw std::runtime_error("unknown algorithm");
    }
}
}  // namespace opentxs::identifier

namespace opentxs::identifier
{
IdentifierPrivate::IdentifierPrivate(
    const identifier::Algorithm algorithm,
    const identifier::Type type,
    const ReadView hash,
    const identifier::AccountSubtype subtype,
    allocator_type alloc) noexcept
    : ByteArrayPrivate(alloc)
    , algorithm_(algorithm)
    , type_(type)
    , account_subtype_(subtype)
{
    Concatenate(hash);
}

IdentifierPrivate::IdentifierPrivate(
    const IdentifierPrivate& rhs,
    allocator_type alloc) noexcept
    : ByteArrayPrivate(rhs, alloc)
    , algorithm_(rhs.algorithm_)
    , type_(rhs.type_)
    , account_subtype_(rhs.account_subtype_)
{
}

auto IdentifierPrivate::AccountType() const noexcept -> opentxs::AccountType
{
    using enum opentxs::AccountType;
    using enum AccountSubtype;
    const auto map =
        frozen::make_unordered_map<AccountSubtype, opentxs::AccountType>({
            {invalid_subtype, Error},
            {custodial_account, Custodial},
            {blockchain_account, Blockchain},
            {blockchain_subaccount, Blockchain},
            {blockchain_subchain, Blockchain},
        });

    if (const auto* i = map.find(account_subtype_); map.end() != i) {

        return i->second;
    } else {

        return Error;
    }
}

auto IdentifierPrivate::asBase58(const api::Crypto& api) const
    -> UnallocatedCString
{
    return text(api).str();
}

auto IdentifierPrivate::asBase58(const api::Crypto& api, alloc::Default alloc)
    const -> CString
{
    return CString{text(api, alloc).str().c_str(), alloc};
}

auto IdentifierPrivate::text(const api::Crypto& api, alloc::Default alloc)
    const noexcept -> std::stringstream
{
    // TODO c++20 use allocator
    auto ss = std::stringstream{};
    const auto required = identifier_expected_hash_bytes(algorithm_);

    if (const auto len = size(); len != required) {
        if (0_uz != len) {
            LogError()()("Incorrect hash size (")(len)(") vs required (")(
                required)(")")
                .Flush();
        }

        return ss;
    }

    const auto preimage = [&] {
        auto out = ByteArray{alloc};
        const auto payload = size();
        const auto haveSubtype = serialize_account_subtype();

        if (0_uz == payload) { return out; }

        const auto type = boost::endian::little_uint16_buf_t{
            static_cast<std::uint16_t>(type_)};
        const auto subtype = boost::endian::little_uint16_buf_t{
            static_cast<std::uint16_t>(account_subtype_)};
        const auto bytes = sizeof(algorithm_) + sizeof(type) + payload +
                           (haveSubtype ? sizeof(subtype) : 0_uz);
        out.resize(bytes);

        assert_true(out.size() == bytes);

        auto* i = static_cast<std::byte*>(out.data());
        std::memcpy(i, &algorithm_, sizeof(algorithm_));
        std::advance(i, sizeof(algorithm_));
        std::memcpy(i, static_cast<const void*>(&type), sizeof(type));
        std::advance(i, sizeof(type));
        std::memcpy(i, data(), payload);
        std::advance(i, payload);

        if (haveSubtype) {
            std::memcpy(i, static_cast<const void*>(&subtype), sizeof(subtype));
            std::advance(i, sizeof(subtype));
        }

        return out;
    }();

    if (0_uz < preimage.size()) {
        ss << identifier_prefix_;
        auto encoded = CString{alloc};

        if (api.Encode().Base58CheckEncode(preimage.Bytes(), writer(encoded))) {
            ss << encoded;
        } else {

            return std::stringstream{};
        }
    }

    return ss;
}

auto IdentifierPrivate::Serialize(protobuf::Identifier& out) const noexcept
    -> bool
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

    out.set_hash(UnallocatedCString{Bytes()});
    out.set_algorithm(static_cast<std::uint32_t>(algorithm_));
    out.set_type(static_cast<std::uint32_t>(type_));
    using enum AccountSubtype;

    if (invalid_subtype != account_subtype_) {
        out.set_account_subtype(static_cast<std::uint32_t>(account_subtype_));
    }

    return true;
}

auto IdentifierPrivate::Serialize(network::zeromq::Message& out) const noexcept
    -> bool
{
    out.Internal().AddFrame([this] {
        auto p = protobuf::Identifier{};
        Serialize(p);

        return p;
    }());

    return true;
}

auto IdentifierPrivate::serialize_account_subtype() const noexcept -> bool
{
    return (AccountSubtype::invalid_subtype != account_subtype_) &&
           (Type::account == type_);
}
}  // namespace opentxs::identifier
